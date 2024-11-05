#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH 700
#define SCREEN_HEIGHT 700
#define GRID_SIZE 35 // Izgara boyutu her kare için 64x64 piksel olarak ayarlandı
// Fonksiyon bildirimleri (prototipleri)
const char* insan_kahramanlar[] = {"Alparslan", "Fatih_Sultan_Mehmet", "Yavuz_Sultan_Selim", "Mete_Han", "Tugrul_Bey"};
const char* insan_canavarlar[] = {"Ejderha", "Agri_Dagi_Devleri", "Tepegoz", "Karakurt", "Samur"};
const char* ork_kahramanlar[] = {"Goruk_Vahsi", "Thruk_Kemikkiran", "Vrog_Kafakiran", "Ugar_Zalim", "Makrog_Savas_Beyi"};
const char* ork_canavarlar[] = {"Kara_Troll", "Golge_Kurtlari", "Camur_Devleri", "Ates_Iblisi", "Buz_Devleri"};
// Birim struct
typedef struct {
    char type[50];
    int attack;
    int defense;
    int saglik;
    int maxsaglik;
    int kritik_sans;
    SDL_Texture* texture;
    int x;
    int y;
    int sayi;
} Birim;
// Fonksiyon bildirimleri
void animateAttack(SDL_Renderer* renderer, Birim* attacker, Birim* defender);
void attack(Birim* attacker, Birim* defender);

// JSON dosyasını okuma fonksiyonu
char* dosyaOku(const char* dosyaAdi) {
    FILE *fp = fopen(dosyaAdi, "r");
    if (fp == NULL) {
        printf("Dosya açılamadı.\n");
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long dosyaBoyutu = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *icerik = (char*) malloc(dosyaBoyutu + 1);
    fread(icerik, 1, dosyaBoyutu, fp);
    icerik[dosyaBoyutu] = '\0';

    fclose(fp);
    return icerik;
}

// SDL'de texture yükleme fonksiyonu
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* file) {
    SDL_Surface* tempSurface = IMG_Load(file);
    if (!tempSurface) {
        printf("Resim yüklenemedi: %s\n", IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    return texture;
}
// Can barı çizim fonksiyonu
void drawHealthBar(SDL_Renderer* renderer, int x, int y, int saglik, int maxsaglik, int isHuman) {
    const int barWidth = GRID_SIZE;
    const int barHeight = 5;
    const int xOffset = 0; // Çubuğun yatay kaydırması
    const int yOffset = 30; // Çubuğun aşağıda durması için

    // Can oranını hesapla
    float healthRatio = (float)saglik/ maxsaglik;

    // Renk ayarlaması
    Uint8 red = 0;   // Başlangıçta kırmızı sıfır
    Uint8 green = 255; // Başlangıçta yeşil maksimumda
    Uint8 blue = 0;   // Mavi sıfırda kalır

    // Can azaldıkça renk ayarlaması
    if (saglik > 0) {
        red = (Uint8)(255 * (1 - healthRatio));   // Can azaldıkça kırmızı artar
        green = (Uint8)(255 * healthRatio);       // Can azaldıkça yeşil azalır
    } else {
        red = 255;  // Sağlık sıfıra ulaşırsa tamamen kırmızı
        green = 0;  // Yeşil yok
    }

    // Barın arka planı
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Beyaz arka plan
    SDL_Rect background = { x + xOffset, y + yOffset, barWidth, barHeight };
    SDL_RenderFillRect(renderer, &background);

    // Can çubuğunu çiz
    SDL_SetRenderDrawColor(renderer, red, green, blue, 255); // Dinamik renk
    SDL_Rect healthBar = { x + xOffset, y + yOffset, (int)(barWidth * healthRatio), barHeight };
    SDL_RenderFillRect(renderer, &healthBar);
}

// JSON verisini ayrıştırıp struct'lara atama fonksiyonu
void jsonAyristirVeAta(const char* json, Birim* Birims, int* numBirims, SDL_Renderer* renderer) {
    char *jsonKopya = strdup(json);
    char *tok = strtok(jsonKopya, "{},:\"[]");

    int insanIndex = 0, orkIndex = 0;
    const char* humanTextures[] = { "piyadeler.png", "okcular.png", "suvariler.png", "kusatma_makineleri.png" };
    const char* orkTextures[] = { "ork_dovusculeri.png", "mizrakcilar.png", "varg_binicileri.png", "troller.png" };

    while (tok != NULL) {
        // İnsan İmparatorluğu birimleri
        if (strcmp(tok, "piyadeler") == 0 || strcmp(tok, "okcular") == 0 || strcmp(tok, "suvariler") == 0 || strcmp(tok, "kusatma_makineleri") == 0) {
            int i = insanIndex++;
            strcpy(Birims[i].type, tok);
            tok = strtok(NULL, "{}:,\"[]");
            while (tok != NULL && strcmp(tok, "saldiri") != 0) tok = strtok(NULL, "{}:,\"[]");
            Birims[i].attack = atoi(strtok(NULL, "{}:,\"[]"));
            while (tok != NULL && strcmp(tok, "savunma") != 0) tok = strtok(NULL, "{}:,\"[]");
            Birims[i].defense = atoi(strtok(NULL, "{}:,\"[]"));
            while (tok != NULL && strcmp(tok, "saglik") != 0) tok = strtok(NULL, "{}:,\"[]");
            Birims[i].saglik = atoi(strtok(NULL, "{}:,\"[]"));
            Birims[i].maxsaglik = Birims[i].saglik;
            while (tok != NULL && strcmp(tok, "kritik_sans") != 0) tok = strtok(NULL, "{}:,\"[]");
            Birims[i].kritik_sans = atoi(strtok(NULL, "{}:,\"[]"));
            Birims[i].texture = loadTexture(renderer, humanTextures[i]);
            Birims[i].x = 0; // Yatay pozisyon
            Birims[i].y = i; // Sabit Y konumu
            (*numBirims)++;
            // JSON'dan gelen veriyi çıktı al
            //printf("İnsan Türü: %s, Sağlık: %d, Max Sağlık: %d, Saldırı: %d, Savunma: %d, Kritik Şans: %d\n",
                //Birims[i].type, Birims[i].health, Birims[i].maxHealth, Birims[i].attack, Birims[i].defense, Birims[i].kritik_sans);
        }
        // Ork Lejyonu birimleri
        else if (strcmp(tok, "ork_dovusculeri") == 0 || strcmp(tok, "mizrakcilar") == 0 || strcmp(tok, "varg_binicileri") == 0 || strcmp(tok, "troller") == 0) {
            int i = orkIndex + 4; // 4-7 arası tip numarası
            strcpy(Birims[i].type, tok);
            tok = strtok(NULL, "{}:,\"[]");
            while (tok != NULL && strcmp(tok, "saldiri") != 0) tok = strtok(NULL, "{}:,\"[]");
            Birims[i].attack = atoi(strtok(NULL, "{}:,\"[]"));
            while (tok != NULL && strcmp(tok, "savunma") != 0) tok = strtok(NULL, "{}:,\"[]");
            Birims[i].defense = atoi(strtok(NULL, "{}:,\"[]"));
            while (tok != NULL && strcmp(tok, "saglik") != 0) tok = strtok(NULL, "{}:,\"[]");
            Birims[i].saglik = atoi(strtok(NULL, "{}:,\"[]"));
            while (tok != NULL && strcmp(tok, "kritik_sans") != 0) tok = strtok(NULL, "{}:,\"[]");
            Birims[i].kritik_sans = atoi(strtok(NULL, "{}:,\"[]"));
            Birims[i].maxsaglik = Birims[i].saglik;
            Birims[i].texture = loadTexture(renderer, orkTextures[orkIndex]);
            Birims[i].x = 19; // Yatay pozisyon
            Birims[i].y = i - 4; // Sabit Y konumu
            orkIndex++;
            (*numBirims)++;
            // JSON'dan gelen veriyi çıktı al
            //printf("Ork Türü: %s, Sağlık: %d, Max Sağlık: %d, Saldırı: %d, Savunma: %d, Kritik Şans: %d\n",
                //Birims[i].type, Birims[i].health, Birims[i].maxHealth, Birims[i].attack, Birims[i].defense, Birims[i].kritik_sans);
        }
        tok = strtok(NULL, "{}:,\"[]");
    }

    free(jsonKopya);
}
// Askerlerin saldırı animasyonu
void animateAttack(SDL_Renderer* renderer, Birim* attacker, Birim* defender) {
    // Basit bir animasyon ekleyebilirsin, örneğin bir çizgi ile saldırıyı göstermek
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Kırmızı çizgi
    SDL_RenderDrawLine(renderer, attacker->x * GRID_SIZE, attacker->y * GRID_SIZE,
                        defender->x * GRID_SIZE, defender->y * GRID_SIZE);
    SDL_RenderPresent(renderer);
    SDL_Delay(500); // 500ms bekle
}

// Saldırı fonksiyonu
// Saldırı fonksiyonu
void attack(Birim* attacker, Birim* defender) {
    // Kritik vuruş hesapla
    float criticalHitChance = (float) rand() / RAND_MAX; // 0 ile 1 arasında rastgele sayı
    int damage = attacker->attack - defender->defense;

    if (criticalHitChance < (float)attacker->kritik_sans / 100) { // Kritik şansını kullan
        damage *= 2; // Kritik vuruşta hasar iki katına çıkar
        printf("%s kritik vuruş yaptı!\n", attacker->type);
    }

    // Savunma ve sağlığa hasarı uygula
    defender->saglik -= damage;
    if (defender->saglik < 0) {
        defender->saglik = 0; // Sağlık sıfırın altına düşmemeli
    }

    printf("%s birimine saldırı: %d hasar! Kalan sağlık: %d\n", defender->type, damage, defender->health);
}


int tumAskerlerOldu(Birim* Birims, int count) {
    for (int i = 0; i < count; i++) {
        if (Birims[i].saglik > 0) {
            return 0; // Hala canlı asker var
        }
    }
    return 1; // Tüm askerler ölü
}
// Izgara çizim fonksiyonu
void drawGrid(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Grid rengi gri
    for (int i = 0; i < SCREEN_WIDTH; i += GRID_SIZE) {
        SDL_RenderDrawLine(renderer, i, 0, i, SCREEN_HEIGHT);
    }
    for (int i = 0; i < SCREEN_HEIGHT; i += GRID_SIZE) {
        SDL_RenderDrawLine(renderer, 0, i, SCREEN_WIDTH, i);
    }
}
void savasDongusu(SDL_Renderer* renderer, Birim* humans, Birim* orcs, int humanCount, int orcCount) {
    int round = 0;
    while (1) {
        // Her turda sırayla bir insan birimi bir ork birimine saldırır ve tam tersi
        for (int i = 0; i < humanCount && i < orcCount; i++) {
            if (round % 2 == 0) { // İnsanların sırası
                if (humans[i].saglik > 0 && orcs[i].saglik > 0) { // Sağ kalan birimlerle saldırı yapılır
                    animateAttack(renderer, &humans[i], &orcs[i]);
                    attack(&humans[i], &orcs[i]); // İnsanlar orklara saldırır
                }
            } else { // Orkların sırası
                if (humans[i].saglik > 0 && orcs[i].saglik > 0) { // Sağ kalan birimlerle saldırı yapılır
                    animateAttack(renderer, &orcs[i], &humans[i]);
                    attack(&orcs[i], &humans[i]); // Orklar insanlara saldırır
                }
            }

            // Can kayıpları sonrası hayatta kalan asker sayısı
            printf("Tur %d bitti: \n", round + 1);
            tumAskerlerOldu(humans, humanCount);
            tumAskerlerOldu(orcs, orcCount);
        }

        // Eğer tüm askerler öldüyse savaşı bitir
        if (tumAskerlerOldu(humans, humanCount) || tumAskerlerOldu(orcs, orcCount)) {
            printf("Savaş bitti!\n");
            break; // Savaşı sona erdir
        }

        round++; // Her turun sonunda sıra değişir
    }
}



int main(int argc, char* args[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Savaş Simülasyonu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // JSON dosyasını oku
    char* jsonVerisi = dosyaOku("data.json");
    if (jsonVerisi == NULL) {
        return 1;
    }

    // Askerleri tutacak dizi
    Birim Birims[10];
    int numBirims = 0;

    // JSON verilerini ayrıştır ve struct'lara ata
    jsonAyristirVeAta(jsonVerisi, Birims, &numBirims, renderer);
    free(jsonVerisi); // JSON verisini serbest bırak

    // İnsanlar ve Orkları ayır
    Birim humans[5];
    Birim orcs[5];
    int humanCount = 0, orcCount = 0;
    for (int i = 0; i < numBirims; i++) {
        if (strcmp(Birims[i].type, "piyadeler") == 0 || strcmp(Birims[i].type, "okcular") == 0 ||
            strcmp(Birims[i].type, "suvariler") == 0 || strcmp(Birims[i].type, "kusatma_makineleri") == 0) {
            humans[humanCount++] = Birims[i];
        } else {
            orcs[orcCount++] = Birims[i];
        }
    }

    // Ana döngü
    int quit = 0;
SDL_Event e;
while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = 1;
        }

        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
            // Savaş başlatma tuşu
            savasDongusu(renderer, humans, orcs, humanCount, orcCount);
        }
    }

    // Savaşın bitip bitmediğini kontrol et
    if (tumAskerlerOldu(humans, humanCount) || tumAskerlerOldu(orcs, orcCount)) {
        printf("Savaş bitti!\n");
        break; // Savaşı sona erdir
    }

    // Ekranı temizle
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Izgarayı çiz
    drawGrid(renderer);

    // Askerleri çiz
    for (int i = 0; i < humanCount; i++) {
        SDL_Rect dstrect = { humans[i].x * GRID_SIZE, humans[i].y * GRID_SIZE, GRID_SIZE, GRID_SIZE };
        SDL_RenderCopy(renderer, humans[i].texture, NULL, &dstrect);
        drawHealthBar(renderer, dstrect.x, dstrect.y, humans[i].health, humans[i].maxHealth, 1);
    }
    for (int i = 0; i < orcCount; i++) {
        SDL_Rect dstrect = { orcs[i].x * GRID_SIZE, orcs[i].y * GRID_SIZE, GRID_SIZE, GRID_SIZE };
        SDL_RenderCopy(renderer, orcs[i].texture, NULL, &dstrect);
        drawHealthBar(renderer, dstrect.x, dstrect.y, orcs[i].health, orcs[i].maxHealth, 0);
    }

    // Ekranı güncelle
    SDL_RenderPresent(renderer);
}

    // SDL kaynaklarını temizle
    for (int i = 0; i < numBirims; i++) {
        SDL_DestroyTexture(Birims[i].texture);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
