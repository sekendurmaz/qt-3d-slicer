---
name: qt-portfolio-project-hardening
overview: Qt/C++ projesini profesyonel bir GitHub portföy projesine dönüştürmek için dokümantasyon, görünürlük ve genel kaliteyi iyileştirme planı.
todos:
  - id: gather-project-details
    content: Projenin temel özelliklerini, teknolojilerini ve hedef platformlarını çıkar
    status: pending
  - id: write-readme
    content: Kapsamlı ve profesyonel bir README.md dosyası oluştur veya güncelle
    status: pending
  - id: create-docs-structure
    content: docs/ klasörü ile mimari ve kullanım dokümantasyonunu hazırla
    status: pending
  - id: capture-screenshots
    content: Uygulamadan ekran görüntüleri ve mümkünse kısa GIF kaydet
    status: pending
  - id: light-code-polish
    content: Önemli sınıflar ve fonksiyonlarda hafif yorum/refaktör iyileştirmeleri yap
    status: pending
  - id: standardize-build
    content: Build talimatlarını README ve gerekirse scriptler ile standardize et
    status: pending
  - id: setup-basic-ci
    content: GitHub Actions ile temel build workflow’u ekle
    status: pending
  - id: curate-git-history
    content: Değişiklikleri anlamlı commit mesajlarıyla düzenli olarak işle
    status: pending
isProject: false
---

## Amaç

Bu plan, mevcut Qt/C++ projeni GitHub’da CV’lik, profesyonel görünümlü bir portföy projesine dönüştürmek için atılacak adımları tanımlar. Odak, **README/dokümantasyon**, **ekran görüntüleri/gifler** ve **genel proje sunumu** üzerindedir; kod kalitesi ve testler için de hafif düzenlemeler içerir.

## Yapılacak Ana Çalışmalar

- **1. Proje özelliklerini netleştirme**
  - Projenin ne yaptığını, hangi teknolojileri kullandığını, hangi problemleri çözdüğünü ve öne çıkan özelliklerini madde madde çıkar.
  - Hedef platform(lar)ı belirt (Windows, Linux, macOS) ve Qt sürümünü not et.
- **2. Profesyonel README tasarla**
  - Proje kök dizinine `[README.md](README.md)` ekle veya mevcutsa içeriğini yeniden yapılandır.
  - Aşağıdaki bölümleri ekle:
    - **Proje başlığı ve kısa tanım** (1–2 cümle, vurucu)
    - **Ekran görüntüleri / GIF**: `docs/` veya `assets/` içinde saklanan görsellerle birkaç arayüz görüntüsü ekle.
    - **Özellikler**: Madde madde önemli özellikler (örn. dinamik layout, tema desteği, dosya yönetimi vs.).
    - **Teknolojiler**: Qt, C++, kullanılan kütüphaneler, CMake/qmake vs.
    - **Kurulum ve Çalıştırma**: Adım adım build ve run talimatları (örn. `git clone`, `cmake ..`, `cmake --build .`, `MyQtApp.exe` ile çalıştırma vb.).
    - **Proje Mimarisi / Klasör Yapısı**: Önemli klasörleri ve ne işe yaradıklarını kısaca açıkla (örn. `src/core`, `src/ui`, `resources` vs.).
    - **Gelecek Geliştirmeler / Roadmap**: Eklenebilecek özellikler (senin vizyonunu gösterir).
    - **Lisans**: Uygun bir açık kaynak lisansı (örn. MIT) seç ve belirt.
- **3. Basit proje dokümantasyonu oluştur**
  - `docs/` klasörü ekleyerek daha detaylı bilgileri buraya koy:
    - `[docs/architecture.md](docs/architecture.md)`: Genel mimari, temel sınıflar ve bileşenler arası etkileşim.
    - `[docs/usage.md](docs/usage.md)`: Uygulama içi temel senaryolar (örneğin sahne oluşturma, mesh ekleme, vs.).
    - Gerekirse kısa diyagramlar (mermaid ile akış diyagramları) ekle.
- **4. Ekran görüntüleri ve demo hazırlığı**
  - Uygulamayı farklı önemli durumlarda (ör. ana ekran, önemli bir işlev çalışırken, ayarlar penceresi vs.) ekran görüntüleriyle belgele.
  - `docs/images/` altında PNG/JPEG olarak sakla.
  - İmkan varsa kısa bir GIF kaydet ve README’de göster (örn. `docs/images/demo.gif`).
- **5. Kod seviyesinde hafif profesyonelleştirme**
  - Önemli sınıflar ve modüller için kısa üst seviye yorumlar ekle (ne işe yaradıkları, sorumlulukları).
  - `namespace` ve `include` düzenini gözden geçir, gereksiz `#include`ları temizle.
  - `TODO` ve geçici logları azalt veya açıklayıcı hale getir.
  - Kritik fonksiyonlar için kısa açıklayıcı yorumlar ekle (özellikle geometri/mesh kısımlarında).
- **6. Build talimatlarını standardize et**
  - Projede CMake kullanıyorsan `[CMakeLists.txt](CMakeLists.txt)` içeriğini README ile uyumlu hale getir (Qt sürümü, minimum C++ standardı vs. belirterek).
  - Gerekirse `scripts/` klasöründe basit build scriptleri oluştur (örn. `[scripts/build_windows.ps1](scripts/build_windows.ps1)`).
  - Geliştirici için minimum gereksinimleri belirt (Qt versiyonu, CMake versiyonu, derleyici).
- **7. Basit kalite kontrol (opsiyonel ama tavsiye)**
  - En azından proje build olup olmamasını kontrol eden çok basit bir GitHub Actions workflow ekle:
    - `[.github/workflows/build.yml](.github/workflows/build.yml)` içinde: repo checkout, Qt kurulumu (mümkünse), CMake configure+build adımları.
  - Ya da daha hafif bir adım olarak: sadece C++ kodunun derlenebilirliğini kontrol eden basic job.
- **8. Git geçmişini düzenli kullan**
  - Bundan sonra yapacağın her mantıklı değişikliği anlamlı commit mesajlarıyla ayır.
  - README/dokümantasyon değişikliklerini, kod refaktöründen ayrı commitle.
  - İstersen GitHub repo açıklamasına ve `Topics` kısmına (örn. `qt`, `c++`, `desktop-app`, `geometry`, `mesh`) etiketler ekle.

## Sonuç

Bu adımlar sonunda GitHub reposu; net bir README, zengin görsel içerik, temel mimari dokümantasyonu ve derlenebilirlik kontrolü ile birlikte profesyonel bir portföy projesi haline gelecek. Öncelik her zaman **anlaşılır ve çekici sunum** olacak, ardından zaman kaldıkça kod kalitesi ve otomasyon tarafı iyileştirilecektir.