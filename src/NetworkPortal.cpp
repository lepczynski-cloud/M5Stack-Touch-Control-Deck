#include "NetworkPortal.h"

#include <Esp.h>

#include "UserConfig.h"

namespace {
const IPAddress AP_ADDRESS(192, 168, 4, 1);
const IPAddress AP_GATEWAY(192, 168, 4, 1);
const IPAddress AP_SUBNET(255, 255, 255, 0);
constexpr uint32_t WIFI_RETRY_INTERVAL_MS = 30000UL;
}

NetworkPortal::NetworkPortal(ConfigManager& configManager, TimeManager& timeManager)
    : configManager_(configManager), timeManager_(timeManager) {}

void NetworkPortal::begin() {
    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);
    registerRoutes();

    const auto& config = configManager_.data();
    if (config.wifiEnabled && !config.wifiSsid.isEmpty()) {
        startStation();
    } else if (config.wifiSsid.isEmpty()) {
        // Przy pierwszym uruchomieniu nie ma jeszcze danych sieci domowej.
        // Uruchamiamy AP, aby mozna bylo otworzyc portal konfiguracji.
        startAccessPoint();
    } else {
        applyWifiMode();
    }
}

void NetworkPortal::loop() {
    const uint32_t now = millis();

    if (apActive_) {
        dnsServer_.processNextRequest();
        server_.handleClient();
    }

    if (pendingRestartMillis_ != 0 && static_cast<int32_t>(now - pendingRestartMillis_) >= 0) {
        ESP.restart();
    }

    if (pendingApStopMillis_ != 0 && static_cast<int32_t>(now - pendingApStopMillis_) >= 0) {
        pendingApStopMillis_ = 0;
        stopAccessPoint();
    }

    if (wifiAttempting_) {
        if (WiFi.status() == WL_CONNECTED) {
            wifiAttempting_ = false;
            nextWifiRetryMillis_ = now + WIFI_RETRY_INTERVAL_MS;
        } else if (now - wifiAttemptStartedMillis_ >= UserDefaults::WIFI_CONNECT_TIMEOUT_MS) {
            wifiAttempting_ = false;
            nextWifiRetryMillis_ = now + WIFI_RETRY_INTERVAL_MS;
            startAccessPoint();
        }
    }

    const auto& config = configManager_.data();
    if (
        config.wifiEnabled &&
        !config.wifiSsid.isEmpty() &&
        WiFi.status() != WL_CONNECTED &&
        !wifiAttempting_ &&
        static_cast<int32_t>(now - nextWifiRetryMillis_) >= 0
    ) {
        startStation();
    }

    // AP wylacza sie dopiero, gdy przez 10 minut nie bylo zapytania HTTP
    // oraz nie jest do niego podlaczony zaden klient.
    if (
        apActive_ &&
        WiFi.softAPgetStationNum() == 0 &&
        now - lastPortalActivityMillis_ >= UserDefaults::AP_IDLE_TIMEOUT_MS
    ) {
        stopAccessPoint();
    }
}

bool NetworkPortal::isWifiConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

bool NetworkPortal::isWifiEnabled() const {
    return configManager_.data().wifiEnabled;
}

bool NetworkPortal::isApActive() const {
    return apActive_;
}

bool NetworkPortal::isWifiConnecting() const {
    return wifiAttempting_;
}

String NetworkPortal::wifiSsid() const {
    if (isWifiConnected()) {
        return WiFi.SSID();
    }
    return configManager_.data().wifiSsid;
}

String NetworkPortal::wifiIp() const {
    return isWifiConnected() ? WiFi.localIP().toString() : String("-");
}

String NetworkPortal::apSsid() const {
    return configManager_.data().apSsid;
}

String NetworkPortal::apIp() const {
    return AP_ADDRESS.toString();
}

String NetworkPortal::webUsername() const {
    return configManager_.data().webUsername;
}

void NetworkPortal::setWifiEnabled(bool enabled) {
    auto& config = configManager_.data();
    if (config.wifiEnabled == enabled) {
        return;
    }

    config.wifiEnabled = enabled;
    configManager_.save();

    if (enabled) {
        if (!config.wifiSsid.isEmpty()) {
            startStation();
        } else {
            startAccessPoint();
        }
    } else {
        stopStation();
    }
}

void NetworkPortal::setApEnabled(bool enabled) {
    if (enabled) {
        startAccessPoint();
    } else {
        stopAccessPoint();
    }
}

void NetworkPortal::reconnectWifi() {
    stopStation();
    delay(100);
    if (configManager_.data().wifiEnabled && !configManager_.data().wifiSsid.isEmpty()) {
        startStation();
    }
}

void NetworkPortal::startStation() {
    const auto& config = configManager_.data();
    if (!config.wifiEnabled || config.wifiSsid.isEmpty()) {
        return;
    }

    WiFi.mode(apActive_ ? WIFI_AP_STA : WIFI_STA);
    WiFi.disconnect(false, false);
    delay(50);
    WiFi.begin(config.wifiSsid.c_str(), config.wifiPassword.c_str());

    wifiAttempting_ = true;
    wifiAttemptStartedMillis_ = millis();
}

void NetworkPortal::stopStation() {
    wifiAttempting_ = false;
    WiFi.disconnect(false, false);
    applyWifiMode();
}

void NetworkPortal::startAccessPoint() {
    if (apActive_) {
        lastPortalActivityMillis_ = millis();
        return;
    }

    const auto& config = configManager_.data();
    WiFi.mode(config.wifiEnabled ? WIFI_AP_STA : WIFI_AP);
    WiFi.softAPConfig(AP_ADDRESS, AP_GATEWAY, AP_SUBNET);

    if (!WiFi.softAP(config.apSsid.c_str(), config.apPassword.c_str())) {
        return;
    }

    dnsServer_.start(53, "*", AP_ADDRESS);
    server_.begin();
    serverRunning_ = true;
    apActive_ = true;
    lastPortalActivityMillis_ = millis();
}

void NetworkPortal::stopAccessPoint() {
    if (!apActive_) {
        return;
    }

    dnsServer_.stop();
    if (serverRunning_) {
        server_.stop();
        serverRunning_ = false;
    }
    WiFi.softAPdisconnect(true);
    apActive_ = false;
    applyWifiMode();
}

void NetworkPortal::applyWifiMode() {
    const auto& config = configManager_.data();
    if (apActive_) {
        WiFi.mode(config.wifiEnabled ? WIFI_AP_STA : WIFI_AP);
    } else if (config.wifiEnabled) {
        WiFi.mode(WIFI_STA);
    } else {
        WiFi.mode(WIFI_OFF);
    }
}

void NetworkPortal::registerRoutes() {
    if (routesRegistered_) {
        return;
    }

    server_.on("/", HTTP_GET, [this]() { handleRoot(); });
    server_.on("/save", HTTP_POST, [this]() { handleSave(); });
    server_.on("/status", HTTP_GET, [this]() { handleStatus(); });
    server_.on("/restart", HTTP_POST, [this]() { handleRestart(); });
    server_.on("/disable-ap", HTTP_POST, [this]() { handleDisableAp(); });
    server_.on("/factory-reset", HTTP_POST, [this]() { handleFactoryReset(); });
    server_.onNotFound([this]() { handleNotFound(); });
    routesRegistered_ = true;
}

bool NetworkPortal::authenticate() {
    markActivity();
    const auto& config = configManager_.data();
    if (server_.authenticate(config.webUsername.c_str(), config.webPassword.c_str())) {
        return true;
    }

    server_.requestAuthentication(BASIC_AUTH, "M5 Control Deck");
    return false;
}

void NetworkPortal::markActivity() {
    lastPortalActivityMillis_ = millis();
}

void NetworkPortal::handleRoot() {
    if (!authenticate()) {
        return;
    }

    server_.sendHeader("Cache-Control", "no-store");
    server_.send(200, "text/html; charset=utf-8", buildPortalHtml());
}

void NetworkPortal::handleSave() {
    if (!authenticate()) {
        return;
    }

    auto& config = configManager_.data();
    const DeviceConfig previousConfig = config;
    String errors;

    config.wifiEnabled = server_.hasArg("wifi_enabled");
    config.wifiSsid = safeFormValue(server_.arg("wifi_ssid"), 32);

    const String newWifiPassword = safeFormValue(server_.arg("wifi_password"), 63);
    if (!newWifiPassword.isEmpty()) {
        config.wifiPassword = newWifiPassword;
    }

    config.brightness = static_cast<uint8_t>(
        constrain(static_cast<int>(server_.arg("brightness").toInt()), 20, 255)
    );

    const int timezoneIndex = server_.arg("timezone").toInt();
    if (timezoneIndex >= 0 && timezoneIndex < static_cast<int>(TimeManager::TIMEZONE_COUNT)) {
        config.secondTimezoneIndex = static_cast<uint8_t>(timezoneIndex);
    }

    config.standbyEnabled = server_.hasArg("standby_enabled");
    config.standbyTimeoutMinutes = static_cast<uint16_t>(
        constrain(static_cast<int>(server_.arg("standby_minutes").toInt()), 1, 60)
    );

    const String newApSsid = safeFormValue(server_.arg("ap_ssid"), 32);
    if (!newApSsid.isEmpty()) {
        config.apSsid = newApSsid;
    }

    const String newApPassword = safeFormValue(server_.arg("ap_password"), 63);
    if (!newApPassword.isEmpty()) {
        if (newApPassword.length() < 8) {
            errors += "Hasło AP musi mieć co najmniej 8 znaków.<br>";
        } else {
            config.apPassword = newApPassword;
        }
    }

    const String newWebUsername = safeFormValue(server_.arg("web_username"), 24);
    if (!newWebUsername.isEmpty()) {
        config.webUsername = newWebUsername;
    }

    const String newWebPassword = safeFormValue(server_.arg("web_password"), 63);
    if (!newWebPassword.isEmpty()) {
        config.webPassword = newWebPassword;
    }

    for (size_t index = 0; index < config.apps.size(); ++index) {
        const String labelArg = "app_label_" + String(index);
        const String urlArg = "app_url_" + String(index);

        const String label = safeFormValue(server_.arg(labelArg), 23);
        const String url = safeFormValue(server_.arg(urlArg), 159);

        if (!label.isEmpty()) {
            config.apps[index].label = label;
        }
        if (!url.isEmpty()) {
            if (isAllowedUrl(url)) {
                config.apps[index].url = url;
            } else {
                errors += "Adres dla kafelka " + String(index + 1) + " musi zaczynać się od http:// lub https://.<br>";
            }
        }
    }

    if (!errors.isEmpty()) {
        // Nie pozostawiamy w pamieci czesciowo zmienionej konfiguracji,
        // gdy formularz zawiera bledny adres lub zbyt krotkie haslo AP.
        config = previousConfig;
        server_.send(
            400,
            "text/html; charset=utf-8",
            "<!doctype html><meta charset='utf-8'><h2>Nie zapisano ustawień</h2><p>" +
                errors + "</p><p><a href='/'>Wróć do formularza</a></p>"
        );
        return;
    }

    configManager_.save();
    M5.Display.setBrightness(config.brightness);

    server_.send(
        200,
        "text/html; charset=utf-8",
        "<!doctype html><meta charset='utf-8'><meta name='viewport' content='width=device-width'>"
        "<style>body{font-family:system-ui;max-width:620px;margin:40px auto;padding:20px}</style>"
        "<h2>Ustawienia zapisane</h2><p>Urządzenie uruchomi się ponownie i zastosuje nową konfigurację.</p>"
    );
    pendingRestartMillis_ = millis() + 1500;
}

void NetworkPortal::handleStatus() {
    if (!authenticate()) {
        return;
    }

    String json = "{";
    json += "\"wifi_enabled\":" + String(configManager_.data().wifiEnabled ? "true" : "false") + ",";
    json += "\"wifi_connected\":" + String(isWifiConnected() ? "true" : "false") + ",";
    json += "\"wifi_ssid\":\"" + jsonEscape(wifiSsid()) + "\",";
    json += "\"wifi_ip\":\"" + jsonEscape(wifiIp()) + "\",";
    json += "\"ap_active\":" + String(apActive_ ? "true" : "false") + ",";
    json += "\"ap_ip\":\"" + jsonEscape(apIp()) + "\"";
    json += "}";

    server_.send(200, "application/json; charset=utf-8", json);
}

void NetworkPortal::handleRestart() {
    if (!authenticate()) {
        return;
    }

    server_.send(200, "text/html; charset=utf-8", "<meta charset='utf-8'><h2>Restart...</h2>");
    pendingRestartMillis_ = millis() + 1000;
}

void NetworkPortal::handleDisableAp() {
    if (!authenticate()) {
        return;
    }

    server_.send(
        200,
        "text/html; charset=utf-8",
        "<meta charset='utf-8'><h2>Punkt AP zostanie wyłączony</h2><p>Połączenie z tą stroną zostanie przerwane.</p>"
    );
    pendingApStopMillis_ = millis() + 1200;
}

void NetworkPortal::handleFactoryReset() {
    if (!authenticate()) {
        return;
    }

    configManager_.resetToDefaults(true);
    server_.send(
        200,
        "text/html; charset=utf-8",
        "<meta charset='utf-8'><h2>Przywrócono ustawienia domyślne</h2><p>Urządzenie uruchomi się ponownie.</p>"
    );
    pendingRestartMillis_ = millis() + 1200;
}

void NetworkPortal::handleNotFound() {
    markActivity();
    server_.sendHeader("Location", "http://192.168.4.1/", true);
    server_.send(302, "text/plain", "");
}

String NetworkPortal::buildPortalHtml() const {
    const auto& config = configManager_.data();

    String html;
    html.reserve(13000);
    html += F("<!doctype html><html lang='pl'><head><meta charset='utf-8'>");
    html += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
    html += F("<title>M5 Control Deck</title><style>");
    html += F(":root{color-scheme:dark;--bg:#0c1220;--card:#172033;--line:#2c3b57;--accent:#36a5ff;--text:#edf5ff;--muted:#a7b4c8}");
    html += F("*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--text);font:16px system-ui,sans-serif}");
    html += F("main{max-width:850px;margin:auto;padding:22px}h1{font-size:1.8rem;margin:0 0 6px}h2{margin:0 0 16px;font-size:1.15rem}");
    html += F(".sub{color:var(--muted);margin:0 0 22px}.status{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:10px;margin-bottom:18px}");
    html += F(".pill,.card{background:var(--card);border:1px solid var(--line);border-radius:14px;padding:16px}.pill b{display:block;margin-bottom:4px}");
    html += F(".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(250px,1fr));gap:14px}.wide{grid-column:1/-1}");
    html += F("label{display:block;color:var(--muted);font-size:.9rem;margin:12px 0 6px}input,select{width:100%;padding:11px;border-radius:9px;border:1px solid var(--line);background:#0f1728;color:var(--text)}");
    html += F("input[type=checkbox]{width:auto;margin-right:8px}.check{display:flex;align-items:center;color:var(--text);margin:10px 0}");
    html += F("button{border:0;border-radius:10px;padding:12px 16px;font-weight:700;cursor:pointer;background:var(--accent);color:#07121f}");
    html += F("button.secondary{background:#283750;color:var(--text)}button.danger{background:#ef6a6a;color:#240606}.actions{display:flex;gap:10px;flex-wrap:wrap;margin-top:18px}");
    html += F("small{color:var(--muted)}code{background:#0a1020;padding:2px 6px;border-radius:6px}@media(max-width:520px){main{padding:15px}.grid{grid-template-columns:1fr}}");
    html += F("</style></head><body><main>");
    html += F("<h1>M5Stack Touch Control Deck</h1><p class='sub'>Lokalny panel konfiguracji. Dane nie są wysyłane do zewnętrznej chmury.</p>");

    html += F("<section class='status'>");
    html += F("<div class='pill'><b>Wi-Fi</b>");
    html += isWifiConnected() ? "Połączono z " + htmlEscape(WiFi.SSID()) : "Brak połączenia";
    html += F("</div><div class='pill'><b>Adres urządzenia</b>");
    html += isWifiConnected() ? WiFi.localIP().toString() : String("-");
    html += F("</div><div class='pill'><b>Punkt AP</b>");
    html += apActive_ ? htmlEscape(config.apSsid) + " / " + AP_ADDRESS.toString() : String("Wyłączony");
    html += F("</div></section>");

    html += F("<form method='post' action='/save'><div class='grid'>");

    html += F("<section class='card'><h2>Sieć i ekran</h2>");
    html += F("<label class='check'><input type='checkbox' name='wifi_enabled' value='1'");
    if (config.wifiEnabled) html += F(" checked");
    html += F(">Łącz z domową siecią Wi-Fi</label>");
    html += F("<label>Nazwa sieci Wi-Fi</label><input name='wifi_ssid' maxlength='32' value='");
    html += htmlEscape(config.wifiSsid);
    html += F("'>");
    html += F("<label>Hasło Wi-Fi</label><input type='password' name='wifi_password' maxlength='63' placeholder='Pozostaw puste, aby nie zmieniać'>");
    html += F("<label>Jasność ekranu: <output id='bv'>");
    html += String(config.brightness);
    html += F("</output></label><input type='range' min='20' max='255' name='brightness' value='");
    html += String(config.brightness);
    html += F("' oninput=\"bv.value=this.value\">");
    html += F("<label>Druga strefa czasowa</label><select name='timezone'>");
    for (size_t index = 0; index < TimeManager::TIMEZONE_COUNT; ++index) {
        html += F("<option value='");
        html += String(index);
        html += F("'");
        if (index == config.secondTimezoneIndex) html += F(" selected");
        html += F(">");
        html += htmlEscape(TimeManager::timezone(index).label);
        html += F(" (");
        html += TimeManager::timezone(index).shortLabel;
        html += F(")</option>");
    }
    html += F("</select>");
    html += F("<label class='check'><input type='checkbox' name='standby_enabled' value='1'");
    if (config.standbyEnabled) html += F(" checked");
    html += F(">Pytaj o tryb czuwania, gdy Bluetooth jest rozłączony</label>");
    html += F("<label>Czas bezczynności przed pytaniem</label><select name='standby_minutes'>");
    const uint16_t standbyOptions[] = {1, 2, 5, 10, 15, 30, 60};
    for (const uint16_t minutes : standbyOptions) {
        html += F("<option value='");
        html += String(minutes);
        html += F("'");
        if (minutes == config.standbyTimeoutMinutes) html += F(" selected");
        html += F(">");
        html += String(minutes);
        html += minutes == 1 ? F(" minuta") : F(" minut");
        html += F("</option>");
    }
    html += F("</select><small>Po odliczeniu wyłączany jest tylko ekran. Bluetooth i Wi-Fi pozostają aktywne.</small></section>");

    html += F("<section class='card'><h2>Punkt AP i logowanie</h2>");
    html += F("<label>Nazwa punktu AP</label><input name='ap_ssid' maxlength='32' value='");
    html += htmlEscape(config.apSsid);
    html += F("'>");
    html += F("<label>Nowe hasło AP</label><input type='password' name='ap_password' maxlength='63' placeholder='Minimum 8 znaków, puste = bez zmian'>");
    html += F("<label>Login do panelu</label><input name='web_username' maxlength='24' value='");
    html += htmlEscape(config.webUsername);
    html += F("'>");
    html += F("<label>Nowe hasło panelu</label><input type='password' name='web_password' maxlength='63' placeholder='Puste = bez zmian'>");
    html += F("<small>Panel działa przez lokalne HTTP tylko w sieci AP urządzenia. Po zakończeniu konfiguracji wyłącz AP.</small></section>");

    html += F("<section class='card wide'><h2>Kafelki aplikacji i stron WWW</h2><div class='grid'>");
    for (size_t index = 0; index < config.apps.size(); ++index) {
        html += F("<div><label>Nazwa kafelka ");
        html += String(index + 1);
        html += F("</label><input name='app_label_");
        html += String(index);
        html += F("' maxlength='23' value='");
        html += htmlEscape(config.apps[index].label);
        html += F("'><label>Adres URL</label><input name='app_url_");
        html += String(index);
        html += F("' maxlength='159' value='");
        html += htmlEscape(config.apps[index].url);
        html += F("'></div>");
    }
    html += F("</div></section></div>");

    html += F("<div class='actions'><button type='submit'>Zapisz i uruchom ponownie</button></div></form>");
    html += F("<div class='actions'><form method='post' action='/restart'><button class='secondary'>Uruchom ponownie</button></form>");
    html += F("<form method='post' action='/disable-ap'><button class='secondary'>Wyłącz punkt AP</button></form>");
    html += F("<form method='post' action='/factory-reset' onsubmit=\"return confirm('Przywrócić ustawienia domyślne?')\"><button class='danger'>Ustawienia fabryczne</button></form></div>");
    html += F("<p><small>Wersja firmware: ");
    html += UserDefaults::FIRMWARE_VERSION;
    html += F("</small></p></main></body></html>");
    return html;
}

String NetworkPortal::htmlEscape(const String& input) {
    String output;
    output.reserve(input.length() + 16);
    for (size_t index = 0; index < input.length(); ++index) {
        switch (input[index]) {
            case '&': output += F("&amp;"); break;
            case '<': output += F("&lt;"); break;
            case '>': output += F("&gt;"); break;
            case '"': output += F("&quot;"); break;
            case '\'': output += F("&#39;"); break;
            default: output += input[index]; break;
        }
    }
    return output;
}

String NetworkPortal::jsonEscape(const String& input) {
    String output;
    output.reserve(input.length() + 16);
    for (size_t index = 0; index < input.length(); ++index) {
        const char character = input[index];
        switch (character) {
            case '\\': output += F("\\\\"); break;
            case '"': output += F("\\\""); break;
            case '\b': output += F("\\b"); break;
            case '\f': output += F("\\f"); break;
            case '\n': output += F("\\n"); break;
            case '\r': output += F("\\r"); break;
            case '\t': output += F("\\t"); break;
            default:
                if (static_cast<uint8_t>(character) >= 0x20) {
                    output += character;
                }
                break;
        }
    }
    return output;
}

String NetworkPortal::safeFormValue(const String& input, size_t maxLength) {
    String output = input;
    output.trim();
    if (output.length() > maxLength) {
        output = output.substring(0, maxLength);
    }
    return output;
}

bool NetworkPortal::isAllowedUrl(const String& url) {
    if (!(url.startsWith("https://") || url.startsWith("http://"))) {
        return false;
    }

    // BLE Keyboard korzysta z mapy standardowych znakow ASCII. Odrzucamy
    // spacje, znaki sterujace i tekst spoza ASCII, aby wpisywanie przez
    // Win+R bylo przewidywalne na ukladzie Polski programisty.
    for (size_t index = 0; index < url.length(); ++index) {
        const uint8_t character = static_cast<uint8_t>(url[index]);
        if (character < 0x21 || character > 0x7E) {
            return false;
        }
    }
    return true;
}
