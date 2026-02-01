🔺 STL (Stereolithography)                                        
   Sadece üçgenlerden oluşur
   ASCII veya Binary olabilir
   Her yüz = 1 üçgen
   Normal + 3 vertex
   
      facet normal nx ny nz
        outer loop
          vertex x y z
          vertex x y z
          vertex x y z
        endloop
      endfacet

🧱 OBJ (Wavefront OBJ)
   Aslında poligon tabanlıdı
   Yüzler:
   Üçgen olabilir
   Dörtgen (quad) olabilir
   Daha fazla köşeli olabilir
   
      Örnek:   
      f 1 2 3
      f 4 5 6 7 
      
      Ama:   
      👉 Slicer veya render motoru objeyi işlerken:   
      Tüm poligonları üçgenlere böler (triangulation)   
      Sonuç:    
      quad → 2 triangle
      pentagon → 3 triangle   
      ➡️ Bellekte yine triangle listesi olur.
      
📦 3MF (3D Manufacturing Format)
   XML tabanlı, modern format
   
   İçinde:   
   Mesh   
   Material   
   Renk  
   Texture   
   Metadata   
   Mesh kısmı:
   <triangle v1="0" v2="1" v3="2"/>  
   ➡️ Yani doğrudan triangle tanımlar

🎯 Özet Tablo
   Format               Dosya İçinde               Program İçinde
   STL                  Triangle                   Triangle
   OBJ                  Polygon / Triangle         Triangle
   3MF                  Triangle                   Triangle


✅ Slicer Perspektifinden 

   Slicer algoritmaları şunu ister:
   std::vector<Triangle>
   
   Dolayısıyla:
   👉 Senin için tek tip iç yapı yeterli:
   
      struct Triangle {
          Vec3 v1;
          Vec3 v2;
          Vec3 v3;
          Vec3 normal;
      };
      
   STL okuyucu → direkt doldurur 
   OBJ okuyucu → poligonları triangulate edip doldurur
   3MF okuyucu → triangle etiketlerinden doldurur
