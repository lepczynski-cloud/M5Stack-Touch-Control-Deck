/* Static bilingual site. No build framework or analytics. The two ESP Web Tools
   manifests and update/factory offsets are intentionally kept unchanged. */
const VIDEO_ID = "Jz1xCBi-Q1o";
const INSTALL_OPTIONS = [
  { manifest: "./firmware/manifest-update.json", image: "m5stack-touch-control-deck.update.bin", offset: 65536 },
  { manifest: "./firmware/manifest.json", image: "m5stack-touch-control-deck.factory.bin", offset: 0 }
];
const translations = {
  en: {
    pageTitle:"M5Stack Touch Control Deck | Build it. Tap it.",metaDesc:"Build and install M5Stack Touch Control Deck: a tiny, wireless Bluetooth controller for Windows, macOS and Linux GNOME. Browser firmware installer included.",
    skip:"Skip to content",navFeatures:"Features",navDemo:"Demo",navInstall:"Install",eyebrow:"OPEN SOURCE · BLUETOOTH HID · ESP32-S3",heroFirst:"Your desk.",heroSecond:"Your shortcuts.",heroThird:"One tap.",
    heroLead:"An open-source touchscreen control deck for the everyday things you do on your computer. No companion software, no soldering, just a little piece of hardware that makes your desk more fun.",heroInstall:"Install firmware",heroDemo:"Watch it in action",compat:"Windows · macOS · Linux GNOME",photoFallback:"View the device photo on GitHub",wireless:"Wireless. No app.",actions:"actions",yourWay:"Set them your way",
    ribbonOne:"6 custom buttons",ribbonTwo:"Media & volume",ribbonThree:"2 time zones",ribbonFour:"Local configuration",featureKicker:"SMALL HARDWARE. BIG CONVENIENCE.",featureTitle:"Everything within reach.",featureIntro:"Six touchscreen pages and a physical knob. Built for the shortcuts you use every day.",
    featSites:"Launch your sites",featSitesBody:"Six editable tiles for ChatGPT, Claude, Gemini, Canva or your own URLs. Faster macOS shortcuts mode is available.",featMedia:"Control the music",featMediaBody:"Play, pause, switch tracks and mute from the screen. Turn the optional Scroll Unit to control volume.",featSystem:"System shortcuts",featSystemBody:"Lock your computer, capture the screen, open your file manager or switch apps. Choose the right host OS.",featLocal:"Yours to configure",featLocalBody:"Set URLs, time zones and brightness through the local Wi-Fi portal. The device works as a Bluetooth keyboard.",
    demoKicker:"SEE THE BUILD",demoTitle:"A project you can actually make.",demoIntro:"The real device, the build process and the finished control deck in action.",playVideo:"Play video",demoSideTitle:"Hardware you can put to work today.",demoSideDesc:"M5Stack CoreS3 SE, an optional Scroll Unit and a 3D-printed stand. You can install the firmware straight from this page.",watchYouTube:"Watch on YouTube ↗",readBuild:"Read the build guide ↗",
    installKicker:"READY TO TRY IT?",installTitle:"From browser to device.",installIntro:"Connect your CoreS3 SE via a USB data cable. No local build or setup required for the web installer.",stepOne:"Connect your M5Stack",stepOneBody:"Use a USB-C cable that supports data, not only charging.",stepTwo:"Choose update or factory install",stepTwoBody:"Update preserves your settings only if you do not choose Erase device. Factory installation resets configuration.",stepThree:"Pair Bluetooth and select your OS",stepThreeBody:"Choose Windows, macOS or Linux GNOME on the device after pairing.",macTipTitle:"On macOS?",macTipText:"For near-instant action triggering, configure native Shortcuts once and enable Fast mode in the device portal.",macTipLink:"Setup guide ↗",
    panelLabel:"WEB INSTALLER",statusChecking:"Checking firmware…",statusReady:"Firmware is ready",statusUnsupported:"Browser doesn't support Web Serial",statusMissing:"Firmware not available",updateTitle:"Already installed?",updateBody:"Update only the application image. Do not choose Erase device to keep your settings.",updateButton:"Update firmware",unsupported:"Use a compatible desktop browser with Web Serial, such as Chrome or Edge.",factoryTitle:"First install or factory reset",factoryBody:"Factory installation overwrites configuration, including Wi-Fi and customized tiles. Choose it only for a new device or an intentional reset.",factoryButton:"Factory install (erase settings)",firmwareMissing:"Firmware files are not available in this deployment. Check the build or compile from the repository.",sourceInstall:"Source installation guide ↗",installWarning:"Only for CoreS3 SE. Updating keeps NVS only if the partition layout is unchanged and Erase device stays disabled.",
    resourcesKicker:"GO FURTHER",resourcesTitle:"Build it your way.",resourcesIntro:"All the parts, instructions and code in one place.",resourceCode:"Code & documentation",resourceTutorial:"Step-by-step build",resourceCase:"3D-printed case by Gigi",resourceMore:"More DIY builds",footerNote:"Independent open-source project by Wojciech Lepczyński. Hardware supported by Botland."
  },
  pl: {
    pageTitle:"M5Stack Touch Control Deck | Zbuduj i wgraj z przeglądarki",metaDesc:"M5Stack Touch Control Deck: bezprzewodowy panel Bluetooth do Windows, macOS i Linux GNOME. Wgraj firmware bezpośrednio z przeglądarki.",
    skip:"Przejdź do treści",navFeatures:"Funkcje",navDemo:"Film",navInstall:"Instalacja",eyebrow:"OPEN SOURCE · BLUETOOTH HID · ESP32-S3",heroFirst:"Twoje biurko.",heroSecond:"Twoje skróty.",heroThird:"Jeden dotyk.",
    heroLead:"Otwarty projekt dotykowego panelu do codziennych czynności na komputerze. Bez aplikacji w tle i bez lutowania. Niewielkie urządzenie, które ułatwia sterowanie komputerem.",heroInstall:"Wgraj firmware",heroDemo:"Zobacz w akcji",compat:"Windows · macOS · Linux GNOME",photoFallback:"Zobacz zdjęcie na GitHubie",wireless:"Bezprzewodowo. Bez aplikacji.",actions:"akcji",yourWay:"Dostosuj do siebie",
    ribbonOne:"6 własnych kafelków",ribbonTwo:"Muzyka i głośność",ribbonThree:"2 strefy czasowe",ribbonFour:"Konfiguracja lokalna",featureKicker:"MAŁY SPRZĘT. DUŻE MOŻLIWOŚCI.",featureTitle:"Wszystko pod ręką.",featureIntro:"Sześć ekranów dotykowych i opcjonalne pokrętło. Twoje ulubione skróty zawsze blisko.",
    featSites:"Otwieraj strony",featSitesBody:"Sześć kafelków z własnymi adresami, np. ChatGPT, Claude, Gemini czy Canva. Na macOS dostępny jest szybszy tryb skrótów.",featMedia:"Steruj multimediami",featMediaBody:"Odtwarzaj, pauzuj, przewijaj i wyciszaj z ekranu. Opcjonalny Scroll Unit pozwala regulować głośność pokrętłem.",featSystem:"Skróty systemowe",featSystemBody:"Blokuj komputer, rób zrzuty ekranu, otwieraj pliki i przełączaj aplikacje. Wybierz odpowiedni system.",featLocal:"Konfiguruj po swojemu",featLocalBody:"Ustawiaj adresy, strefy czasowe i jasność w lokalnym panelu Wi-Fi. Komputer rozpoznaje zwykłą klawiaturę Bluetooth.",
    demoKicker:"ZOBACZ BUDOWĘ",demoTitle:"Projekt, który możesz odtworzyć.",demoIntro:"Prawdziwe urządzenie, montaż i demonstracja działania.",playVideo:"Odtwórz film",demoSideTitle:"Sprzęt gotowy do codziennej pracy.",demoSideDesc:"M5Stack CoreS3 SE, opcjonalny Scroll Unit i podstawka z drukarki 3D. Firmware możesz wgrać bezpośrednio z tej strony.",watchYouTube:"Zobacz na YouTube ↗",readBuild:"Poradnik na Forbocie ↗",
    installKicker:"CHCESZ WYPRÓBOWAĆ?",installTitle:"Z przeglądarki na urządzenie.",installIntro:"Podłącz CoreS3 SE kablem USB z transmisją danych. Instalator nie wymaga lokalnej kompilacji.",stepOne:"Podłącz M5Stack",stepOneBody:"Użyj kabla USB-C obsługującego transmisję danych, nie tylko ładowanie.",stepTwo:"Wybierz aktualizację lub instalację fabryczną",stepTwoBody:"Aktualizacja zachowa ustawienia tylko wtedy, gdy nie wybierzesz Erase device. Instalacja fabryczna usuwa konfigurację.",stepThree:"Sparuj Bluetooth i wybierz system",stepThreeBody:"Po sparowaniu wybierz na urządzeniu Windows, macOS albo Linux GNOME.",macTipTitle:"Korzystasz z macOS?",macTipText:"Aby szybko uruchamiać akcje, skonfiguruj raz systemowe Skróty i włącz tryb Szybki w portalu urządzenia.",macTipLink:"Instrukcja konfiguracji ↗",
    panelLabel:"INSTALATOR WWW",statusChecking:"Sprawdzam firmware…",statusReady:"Firmware gotowy",statusUnsupported:"Przeglądarka nie obsługuje Web Serial",statusMissing:"Brak firmware",updateTitle:"Masz już urządzenie?",updateBody:"Aktualizowana jest tylko aplikacja. Nie wybieraj Erase device, jeśli chcesz zachować ustawienia.",updateButton:"Aktualizuj firmware",unsupported:"Użyj obsługującej Web Serial przeglądarki na komputerze, np. Chrome lub Edge.",factoryTitle:"Pierwsza instalacja lub reset",factoryBody:"Instalacja fabryczna nadpisuje konfigurację, łącznie z Wi-Fi i kafelkami. Użyj jej wyłącznie przy pierwszej instalacji lub celowym resecie.",factoryButton:"Instalacja fabryczna (kasuje ustawienia)",firmwareMissing:"W tym wdrożeniu brakuje firmware. Sprawdź proces budowania albo skompiluj projekt z repozytorium.",sourceInstall:"Instrukcja instalacji ze źródeł ↗",installWarning:"Tylko dla CoreS3 SE. Aktualizacja zachowuje NVS tylko przy tym samym układzie partycji i bez opcji Erase device.",
    resourcesKicker:"DOWIEDZ SIĘ WIĘCEJ",resourcesTitle:"Zbuduj po swojemu.",resourcesIntro:"Części, dokumentacja i kod w jednym miejscu.",resourceCode:"Kod i dokumentacja",resourceTutorial:"Poradnik krok po kroku",resourceCase:"Obudowa 3D autorstwa Gigi",resourceMore:"Więcej projektów DIY",footerNote:"Niezależny projekt open source Wojciecha Lepczyńskiego. Sprzęt dostarczył Botland."
  }
};
let currentLang = "en";
let firmwareState = "checking";
const $ = (id) => document.getElementById(id);
function requestedLanguage() {
  const fromQuery = new URLSearchParams(location.search).get("lang");
  if (fromQuery === "pl" || fromQuery === "en") return fromQuery;
  return (navigator.language || "en").toLowerCase().startsWith("pl") ? "pl" : "en";
}
function translate(lang) {
  currentLang = lang;
  const t = translations[lang];
  document.documentElement.lang = lang;
  document.title = t.pageTitle;
  document.querySelector('meta[name="description"]').content = t.metaDesc;
  document.querySelectorAll("[data-i18n]").forEach((node) => {
    const value = t[node.dataset.i18n];
    if (typeof value === "string") node.textContent = value;
  });
  document.querySelectorAll(".language-switch button").forEach((button) => {
    button.setAttribute("aria-pressed", String(button.dataset.lang === lang));
  });
  renderStatus();
}
function renderStatus() {
  const root = $("firmwareStatus");
  const t = translations[currentLang];
  const status = {checking:"statusChecking",ready:"statusReady",missing:"statusMissing",unsupported:"statusUnsupported"}[firmwareState];
  root.querySelector("span:last-child").textContent = t[status];
  root.querySelector(".status-dot").className = "status-dot " + (firmwareState === "ready" ? "ready" : firmwareState === "checking" ? "pending" : "missing");
}
async function checkFirmware() {
  firmwareState = "checking";
  renderStatus();
  try {
    const checks = await Promise.all(INSTALL_OPTIONS.map(async (item) => {
      const [manifestResponse, imageResponse] = await Promise.all([
        fetch(item.manifest, {cache:"no-store"}),
        fetch(`./firmware/${item.image}`, {method:"HEAD", cache:"no-store"})
      ]);
      if (!manifestResponse.ok || !imageResponse.ok) throw new Error("File unavailable");
      const manifest = await manifestResponse.json();
      const build = manifest.builds?.find(b => b.chipFamily === "ESP32-S3");
      const part = build?.parts?.[0];
      const imageType = (imageResponse.headers.get("content-type") || "").toLowerCase();
      const imageSize = Number(imageResponse.headers.get("content-length") || "0");
      if (!/^\d+\.\d+\.\d+$/.test(manifest.version || "") || !manifest.new_install_prompt_erase ||
          build?.parts?.length !== 1 || part?.path !== item.image || part?.offset !== item.offset ||
          imageType.includes("text/html") || (imageSize > 0 && imageSize < 65536)) throw new Error("Invalid firmware manifest");
      return manifest.version;
    }));
    if (checks[0] !== checks[1]) throw new Error("Mismatched image versions");
    const version = $("install").querySelector(".installer-version");
    version.textContent = `v${checks[0]}`;
    $("installerWrapper").hidden = false;
    $("firmwareMissing").hidden = true;
    firmwareState = "serial" in navigator ? "ready" : "unsupported";
  } catch (_error) {
    $("installerWrapper").hidden = true;
    $("firmwareMissing").hidden = false;
    firmwareState = "missing";
  }
  renderStatus();
}
function setupVideo() {
  $("playVideo").addEventListener("click", () => {
    if (!/^[\w-]{11}$/.test(VIDEO_ID)) return;
    const frame = document.createElement("iframe");
    frame.src = `https://www.youtube-nocookie.com/embed/${VIDEO_ID}?autoplay=1&rel=0`;
    frame.title = "M5Stack Touch Control Deck - build and demo";
    frame.allow = "accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share";
    frame.referrerPolicy = "strict-origin-when-cross-origin";
    frame.allowFullscreen = true;
    $("videoFrame").replaceChildren(frame);
  }, {once:true});
}
function setupPhoto() {
  const photo = $("productPhoto");
  const useFallback = () => {
    if (!photo.dataset.retry) {
      photo.dataset.retry = "1";
      photo.src = "https://raw.githubusercontent.com/lepczynski-cloud/M5Stack-Touch-Control-Deck/main/images/m5stack-touch-control-deck.jpg";
    } else {
      photo.hidden = true;
      $("photoFallback").hidden = false;
    }
  };
  photo.addEventListener("error", useFallback);
  if (photo.complete && !photo.naturalWidth) useFallback();
}
function setupMotion() {
  if (matchMedia("(prefers-reduced-motion: reduce)").matches) return;
  if ("IntersectionObserver" in window) {
    document.documentElement.classList.add("js-motion");
    const observer = new IntersectionObserver((entries) => {
      entries.forEach((item) => {
        if (item.isIntersecting) {
          item.target.classList.add("in-view");
          observer.unobserve(item.target);
        }
      });
    }, {rootMargin:"0px 0px -40px 0px",threshold:0.07});
    document.querySelectorAll(".reveal").forEach((node) => observer.observe(node));
  }
  if (!matchMedia("(pointer:fine)").matches) return;
  const glow = document.querySelector(".cursor-spotlight");
  let x = -600, y = -600, scheduled = false;
  addEventListener("pointermove", (e) => {
    if (e.pointerType === "touch") return;
    x = e.clientX - 220;
    y = e.clientY - 220;
    if (!scheduled) {
      scheduled = true;
      requestAnimationFrame(() => {
        glow.style.transform = `translate3d(${x}px,${y}px,0)`;
        scheduled = false;
      });
    }
  }, {passive:true});
}
function init() {
  translate(requestedLanguage());
  document.querySelectorAll(".language-switch button").forEach((button) => {
    button.addEventListener("click", () => {
      const url = new URL(location.href);
      url.searchParams.set("lang", button.dataset.lang);
      history.replaceState(null, "", url);
      translate(button.dataset.lang);
    });
  });
  addEventListener("popstate", () => translate(requestedLanguage()));
  setupVideo();
  setupPhoto();
  setupMotion();
  checkFirmware();
}
if (document.readyState === "loading") document.addEventListener("DOMContentLoaded", init);
else init();
