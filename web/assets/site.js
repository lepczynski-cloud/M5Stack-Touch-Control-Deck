/* M5Stack Touch Control Deck - static bilingual project website.
 * Full M5Stack build video: https://www.youtube.com/watch?v=Jz1xCBi-Q1o
 */
const YOUTUBE_VIDEO_ID = "Jz1xCBi-Q1o";
const FW_PATH = "./firmware/m5stack-touch-control-deck.factory.bin";
const MANIFEST_PATH = "./firmware/manifest.json";

const translations = {
  en: {
    pageTitle: "M5Stack Touch Control Deck | Install from your browser",
    metaDesc: "Install M5Stack Touch Control Deck firmware in your browser. Open-source wireless touchscreen controller for Windows, powered by M5Stack CoreS3 SE.",
    skip: "Skip to content", navFeatures: "Features", navInstall: "Install", navVideo: "Video", navResources: "Resources",
    eyebrow: "OPEN SOURCE · ESP32-S3 · BLUETOOTH HID", heroLineOne: "Your desktop,", heroLineTwo: "one touch away.",
    heroLead: "A compact, wireless control deck for Windows. Open your favorite tools, control media, and run shortcuts on a beautiful little touchscreen.",
    chipNoApp: "No companion app", chipNoSolder: "No soldering", chipSource: "Open source", buttonInstall: "Install firmware", buttonCode: "View source code ↗",
    heroNote: "Made for M5Stack CoreS3 SE · Windows 10 / 11", photoFallback: "See the finished device on GitHub", photoCaption: "REAL BUILD · 3D PRINTED ENCLOSURE",
    tagWireless: "Wireless connection", tagScreens: "screens", tagCustomizable: "Fully customizable",
    statOne: "6 touchscreen pages", statTwo: "Bluetooth keyboard", statThree: "No host software", statFour: "MIT licensed",
    kickerFeatures: "WHAT IT DOES", featuresTitle: "Small device. Useful shortcuts.", featuresIntro: "One screen for the things you reach for every day. Swipe between pages and make it yours.",
    featAppsTitle: "Your favorite apps", featAppsDesc: "Six editable buttons for websites and everyday tools. Launch a URL with a single touch.",
    featMediaTitle: "Media & volume", featMediaDesc: "Play, pause, skip or mute. The optional Scroll Unit adds a physical volume knob.",
    featShortcutsTitle: "Windows shortcuts", featShortcutsDesc: "Task Manager, screenshots, File Explorer, desktop and lock screen. No extra app required.",
    featClocksTitle: "Two time zones", featClocksDesc: "See local time and another city. Synchronize over Wi-Fi and keep time using the RTC.",
    featBleTitle: "Bluetooth HID", featBleDesc: "Windows recognizes a standard wireless keyboard. No Python service or USB cable in daily use.",
    featConfigTitle: "Local configuration", featConfigDesc: "Change buttons, brightness and time zones through a local web portal on the device.",
    kickerInstall: "WEB FLASHER", installTitle: "From browser to device.", installIntro: "No local compilation for visitors. Just the correct board, a USB data cable and a compatible desktop browser.",
    step1Title: "Connect your device", step1Desc: "Plug your M5Stack CoreS3 SE into your computer with a USB-C data cable. The battery and Scroll Unit are optional.",
    step2Title: "Install in the browser", step2Desc: "Use a compatible desktop browser over HTTPS, click Install, and select your device's USB serial port.",
    step3Title: "Pair with Windows", step3Desc: "After installation, add “M5 Control Deck” in Windows Bluetooth settings and start using your shortcuts.",
    noteTitle: "Before you start", noteText: "Only for M5Stack CoreS3 SE. Flashing changes device firmware and can erase saved settings. The controller's keyboard actions target Windows 10/11.",
    installerLabel: "DEVICE INSTALLER", installerDevice: "M5Stack CoreS3 SE", installerSpec: "ESP32-S3 · 16 MB flash · USB-C", statusChecking: "Checking firmware availability…",
    statusReady: "Firmware ready to install", statusMissing: "Firmware is unavailable in this deployment. Please try again later or see the GitHub installation instructions.",
    statusUnavailable: "Firmware package unavailable", statusUnsupported: "Browser installation unavailable", fallbackInstall: "Use the Windows upload guide ↗", flashButton: "Install firmware",
    unsupported: "Your browser does not support this installer. Open this site in a compatible desktop browser such as Chrome or Edge.",
    installerFooter: "Installation runs locally in your browser over USB. No account required.",
    kickerVideo: "BUILD & DEMO", videoTitle: "See how it comes together.", videoIntro: "A closer look at the hardware, 3D-printed case, firmware upload and the finished control deck in action.",
    videoPlaceholder: "Build video", videoPlaceholderDesc: "The full YouTube video will be embedded here.", videoChannel: "Visit my YouTube channel ↗", videoInside: "IN THE VIDEO",
    videoSideTitle: "A project you can actually build.", videoBullet1: "Assembling the M5Stack modules without soldering", videoBullet2: "Fitting a 3D-printed desk enclosure",
    videoBullet3: "Flashing and starting the firmware", videoBullet4: "Using the touchscreen, knob and Bluetooth shortcuts", youtubeChannel: "More projects on YouTube",
    kickerResources: "GO DEEPER", resourcesTitle: "Build it your way.", resourcesIntro: "Everything you need to understand the project, modify the source code or print the case.",
    resGithub: "Source code & documentation", resGithubSub: "Open source, ready to customize.", resForbot: "Detailed build article", resForbotSub: "Polish guide. Browser translation works well.",
    resCase: "3D-printed desk case", resCaseSub: "An optional model by Gigi, not my design.",
    bottomKicker: "MAKE IT YOURS", bottomTitle: "A small screen with big possibilities.", bottomDesc: "Have an idea for the next feature? The code is open. Contributions and suggestions are welcome.", bottomButton: "Suggest a feature",
    footerLeft: "An independent open-source DIY project by Wojciech Lepczyński.", footerSupport: "Hardware provided by Botland.", footerNote: "Project source: MIT License. 3D case by its respective author."
  },
  pl: {
    pageTitle: "M5Stack Touch Control Deck | Wgrywanie firmware przez przeglądarkę",
    metaDesc: "Wgraj M5Stack Touch Control Deck z przeglądarki. Bezprzewodowy panel dotykowy do Windows na M5Stack CoreS3 SE. Kod open source.",
    skip: "Przejdź do treści", navFeatures: "Funkcje", navInstall: "Instalacja", navVideo: "Film", navResources: "Materiały",
    eyebrow: "OPEN SOURCE · ESP32-S3 · BLUETOOTH HID", heroLineOne: "Twój komputer,", heroLineTwo: "pod jednym dotykiem.",
    heroLead: "Niewielki, bezprzewodowy panel do Windows. Otwieraj ulubione narzędzia, steruj multimediami i uruchamiaj skróty na ekranie dotykowym.",
    chipNoApp: "Bez aplikacji na PC", chipNoSolder: "Bez lutowania", chipSource: "Otwarty kod", buttonInstall: "Wgraj firmware", buttonCode: "Zobacz kod na GitHubie ↗",
    heroNote: "Dla M5Stack CoreS3 SE · Windows 10 / 11", photoFallback: "Zobacz urządzenie na GitHubie", photoCaption: "GOTOWY PROJEKT · OBUDOWA Z DRUKU 3D",
    tagWireless: "Połączenie bezprzewodowe", tagScreens: "ekranów", tagCustomizable: "Możesz je dostosować",
    statOne: "6 ekranów dotykowych", statTwo: "Klawiatura Bluetooth", statThree: "Bez programu na PC", statFour: "Licencja MIT",
    kickerFeatures: "CO POTRAFI", featuresTitle: "Mały panel. Przydatne funkcje.", featuresIntro: "Najczęściej używane narzędzia pod ręką. Przesuwaj palcem między ekranami i dostosuj je do siebie.",
    featAppsTitle: "Ulubione aplikacje", featAppsDesc: "Sześć kafelków z edytowanymi adresami. Otwieraj strony i narzędzia jednym dotknięciem.",
    featMediaTitle: "Muzyka i głośność", featMediaDesc: "Play, pauza, następny utwór i wyciszanie. Opcjonalny Scroll Unit dodaje fizyczne pokrętło.",
    featShortcutsTitle: "Skróty Windows", featShortcutsDesc: "Menedżer zadań, wycinki ekranu, Eksplorator, pulpit i blokada komputera bez dodatkowego programu.",
    featClocksTitle: "Dwie strefy czasowe", featClocksDesc: "Godzina lokalna i w drugim mieście. Synchronizacja przez Wi-Fi i praca z zegarem RTC.",
    featBleTitle: "Bluetooth HID", featBleDesc: "Windows widzi zwykłą klawiaturę Bluetooth. Na co dzień nie potrzebujesz usługi Python ani przewodu USB.",
    featConfigTitle: "Lokalna konfiguracja", featConfigDesc: "Zmieniaj kafelki, jasność i strefy czasowe w lokalnym panelu WWW urządzenia.",
    kickerInstall: "INSTALATOR WWW", installTitle: "Z przeglądarki na urządzenie.", installIntro: "Bez kompilowania po stronie użytkownika. Wystarczy odpowiedni moduł, przewód USB z transmisją danych i zgodna przeglądarka na komputerze.",
    step1Title: "Podłącz urządzenie", step1Desc: "Podłącz M5Stack CoreS3 SE do komputera przewodem USB-C z transmisją danych. Akumulator i pokrętło są opcjonalne.",
    step2Title: "Wgraj przez przeglądarkę", step2Desc: "Otwórz stronę przez HTTPS w zgodnej przeglądarce komputerowej, kliknij Instaluj i wybierz port USB urządzenia.",
    step3Title: "Sparuj z Windows", step3Desc: "Po wgraniu dodaj „M5 Control Deck” w ustawieniach Bluetooth Windows i korzystaj ze skrótów.",
    noteTitle: "Zanim zaczniesz", noteText: "Tylko dla M5Stack CoreS3 SE. Wgrywanie zmienia firmware i może usunąć zapisane ustawienia. Skróty klawiaturowe przeznaczone są dla Windows 10/11.",
    installerLabel: "INSTALATOR URZĄDZENIA", installerDevice: "M5Stack CoreS3 SE", installerSpec: "ESP32-S3 · 16 MB flash · USB-C", statusChecking: "Sprawdzam dostępność firmware…",
    statusReady: "Firmware gotowy do wgrania", statusMissing: "Firmware nie jest dostępny w tym wdrożeniu. Spróbuj ponownie później lub skorzystaj z instrukcji na GitHubie.",
    statusUnavailable: "Brak gotowego firmware", statusUnsupported: "Przeglądarka nie obsługuje instalatora", fallbackInstall: "Instrukcja wgrywania przez Windows ↗", flashButton: "Wgraj firmware",
    unsupported: "Twoja przeglądarka nie obsługuje instalatora. Otwórz stronę na komputerze w zgodnej przeglądarce, np. Chrome lub Edge.",
    installerFooter: "Wgrywanie odbywa się lokalnie przez USB, bez zakładania konta.",
    kickerVideo: "BUDOWA I DEMO", videoTitle: "Zobacz projekt w praktyce.", videoIntro: "Montaż modułów, obudowa 3D, wgrywanie kodu i działanie gotowego panelu.",
    videoPlaceholder: "Film z budowy", videoPlaceholderDesc: "W tym miejscu pojawi się pełny film z YouTube.", videoChannel: "Odwiedź mój kanał YouTube ↗", videoInside: "W FILMIE",
    videoSideTitle: "Projekt, który możesz odtworzyć.", videoBullet1: "Łączenie modułów M5Stack bez lutowania", videoBullet2: "Montaż obudowy wydrukowanej w 3D",
    videoBullet3: "Wgrywanie i uruchamianie firmware", videoBullet4: "Sterowanie ekranem, pokrętłem i skrótami Bluetooth", youtubeChannel: "Więcej projektów na YouTube",
    kickerResources: "DOWIEDZ SIĘ WIĘCEJ", resourcesTitle: "Zbuduj go po swojemu.", resourcesIntro: "Kod, dokumentacja, dokładny poradnik i model obudowy w jednym miejscu.",
    resGithub: "Kod i dokumentacja", resGithubSub: "Open source, możesz modyfikować.", resForbot: "Szczegółowy poradnik budowy", resForbotSub: "Pełny opis po polsku na Forbocie.",
    resCase: "Obudowa do druku 3D", resCaseSub: "Opcjonalny model autora Gigi, nie mój projekt.",
    bottomKicker: "DOSTOSUJ DO SIEBIE", bottomTitle: "Mały ekran, wiele możliwości.", bottomDesc: "Masz pomysł na kolejną funkcję? Kod jest otwarty. Chętnie poznam sugestie i propozycje zmian.", bottomButton: "Zaproponuj funkcję",
    footerLeft: "Niezależny projekt DIY open source autorstwa Wojciecha Lepczyńskiego.", footerSupport: "Sprzęt dostarczył Botland.", footerNote: "Kod: licencja MIT. Obudowa 3D należy do jej autora."
  }
};

let currentLang = "en";
let firmwareState = "checking";

function requestedLanguage() {
  const lang = new URLSearchParams(window.location.search).get("lang");
  if (lang === "pl" || lang === "en") return lang;
  return (navigator.language || "en").toLowerCase().startsWith("pl") ? "pl" : "en";
}

function translate(lang) {
  currentLang = lang;
  const dict = translations[lang];
  document.documentElement.lang = lang;
  document.title = dict.pageTitle;
  document.querySelector('meta[name="description"]').setAttribute("content", dict.metaDesc);
  document.querySelectorAll("[data-i18n]").forEach(node => {
    const value = dict[node.dataset.i18n];
    if (typeof value === "string") node.textContent = value;
  });
  document.querySelectorAll(".language button").forEach(button => {
    button.setAttribute("aria-pressed", String(button.dataset.lang === lang));
  });
  renderFirmwareStatus();
}

function renderFirmwareStatus() {
  const line = document.getElementById("firmwareStatus");
  if (!line) return;
  const t = translations[currentLang];
  const dot = line.querySelector(".status-dot");
  const label = line.querySelector("span:last-child");
  dot.className = "status-dot";
  let key = "statusChecking";
  if (firmwareState === "ready") {key = "statusReady"; dot.classList.add("ready");}
  if (firmwareState === "missing") {key = "statusUnavailable"; dot.classList.add("missing");}
  if (firmwareState === "unsupported") {key = "statusUnsupported"; dot.classList.add("missing");}
  if (firmwareState === "checking") dot.classList.add("pending");
  label.textContent = t[key];
}

async function checkFirmware() {
  const wrapper = document.getElementById("installerWrapper");
  const missing = document.getElementById("firmwareMissing");
  firmwareState = "checking";
  renderFirmwareStatus();
  try {
    const manifestResponse = await fetch(MANIFEST_PATH, {cache: "no-store"});
    if (!manifestResponse.ok) throw new Error("Missing firmware manifest");
    const manifest = await manifestResponse.json();
    const versionLabel = document.querySelector(".installer-version");
    if (versionLabel && /^\d+\.\d+\.\d+$/.test(manifest.version || "")) {
      versionLabel.textContent = `v${manifest.version}`;
    }
    const build = manifest.builds?.find(b => b.chipFamily === "ESP32-S3");
    if (!build || build.parts?.length !== 1 || build.parts[0].offset !== 0 || build.parts[0].path !== "m5stack-touch-control-deck.factory.bin") {
      throw new Error("Firmware manifest does not match the board");
    }
    const response = await fetch(FW_PATH, {method: "HEAD", cache: "no-store"});
    const contentType = (response.headers.get("content-type") || "").toLowerCase();
    const size = Number(response.headers.get("content-length") || 0);
    if (!response.ok || contentType.includes("text/html") || (size > 0 && size < 65536)) throw new Error("Firmware image is missing or invalid");
    if (!("serial" in navigator)) {
      firmwareState = "unsupported";
      wrapper.hidden = false;
    } else {
      firmwareState = "ready";
      wrapper.hidden = false;
    }
    missing.hidden = true;
  } catch (_error) {
    firmwareState = "missing";
    wrapper.hidden = true;
    missing.hidden = false;
  }
  renderFirmwareStatus();
}

function loadVideo() {
  if (!/^[\w-]{11}$/.test(YOUTUBE_VIDEO_ID)) return;
  const frame = document.getElementById("videoFrame");
  const iframe = document.createElement("iframe");
  iframe.src = `https://www.youtube-nocookie.com/embed/${YOUTUBE_VIDEO_ID}?rel=0`;
  iframe.title = "M5Stack Touch Control Deck - build and demonstration";
  iframe.loading = "lazy";
  iframe.referrerPolicy = "strict-origin-when-cross-origin";
  iframe.allow = "accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share";
  iframe.allowFullscreen = true;
  document.getElementById("videoEmpty").remove();
  frame.append(iframe);
}

function setupImageFallback() {
  const photo = document.getElementById("productPhoto");
  const fallback = document.getElementById("photoFallback");
  photo.addEventListener("error", () => {photo.hidden = true; fallback.hidden = false;});
}

function init() {
  translate(requestedLanguage());
  document.querySelectorAll(".language button").forEach(button => {
    button.addEventListener("click", () => {
      const url = new URL(window.location.href);
      url.searchParams.set("lang", button.dataset.lang);
      window.history.replaceState(null, "", url);
      translate(button.dataset.lang);
    });
  });
  window.addEventListener("popstate", () => translate(requestedLanguage()));
  setupImageFallback();
  loadVideo();
  checkFirmware();
}

if (document.readyState === "loading") document.addEventListener("DOMContentLoaded", init);
else init();
