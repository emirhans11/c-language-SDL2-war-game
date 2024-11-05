#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include<math.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <curl/curl.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800
#define GRID_SIZE 40
#define MAX_BIRIMLER 10
#define MAX_ISIM_UZUNLUGU 50
#define MAX_KAHRAMAN 10
#define MAX_CANAVAR 10  
const char* humanTextures[] = {
    "piyadeler.png", 
    "okcular.png", 
    "suvariler.png", 
    "kusatma_makineleri.png"
}; 
const char* orkTextures[] = {
    "ork_dovusculeri.png", 
    "mizrakcilar.png", 
    "varg_binicileri.png", 
    "troller.png"
};
const char* insan_kahramanlar[] = {"Alparslan", "Fatih_Sultan_Mehmet", "Yavuz_Sultan_Selim", "Mete_Han", "Tugrul_Bey"};
const char* insan_canavarlar[] = {"Ejderha", "Agri_Dagi_Devleri", "Tepegoz", "Karakurt", "Samur"};
const char* ork_kahramanlar[] = {"Goruk_Vahsi", "Thruk_Kemikkiran", "Vrog_Kafakiran", "Ugar_Zalim", "Makrog_Savas_Beyi"};
const char* ork_canavarlar[] = {"Kara_Troll", "Golge_Kurtlari", "Camur_Devleri", "Ates_Iblisi", "Buz_Devleri"};
void etkiledigi_birim_ayikla(const char* aciklama, char* etkiledigi_birim); 
void ekranaYaziYaz(SDL_Renderer* renderer, const char* yazi, int x, int y, SDL_Color renk);
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
typedef struct {
    char isim[50];
    float saglik;
    float max_saglik;
    int saldiri;
    int savunma;
    int kritik_sans;
    int sayi; 
} Birim;
typedef struct {
    Birim birimler[10];
    int birim_sayisi;
} InsanIrki;
typedef struct {
    Birim birimler[10];
    int birim_sayisi;
} OrkLegi;
typedef struct {
    char isim[50];
    char bonus_turu[20];
    char etkiledigi_birim[50];
    int bonus_degeri;
    char aciklama[100];
} Kahraman;
typedef struct {
    Kahraman kahramanlar[10];
    int kahraman_sayisi;
} InsanKahramanlari;
typedef struct {
    Kahraman kahramanlar[10];
    int kahraman_sayisi;
} OrkKahramanlari;
typedef struct {
    char isim[50];
    int seviye;
    int deger;
    char aciklama[100];
} Arastirma;
typedef struct {
    Arastirma arastirmalar[20];
    int arastirma_sayisi;
} ArastirmaListesi;
typedef struct {
    char isim[MAX_ISIM_UZUNLUGU];
    int etki_degeri;
    char etki_turu[MAX_ISIM_UZUNLUGU];
    char etkiledigi_birim[MAX_ISIM_UZUNLUGU];
} BirimCanavar;
typedef struct {
    BirimCanavar birimler[MAX_BIRIMLER];
    int birim_sayisi;
} Irk;
typedef struct {
    int piyadeler;
    int okcular;
    int suvariler;
    int kusatma_makineleri;
    char kahramanlar[MAX_KAHRAMAN][30]; // Kahraman isimleri için dizi
    char canavarlar[MAX_CANAVAR][30];   // Canavar isimleri için dizi
    int insan_elit_egitim;
    int insan_saldiri_gelistirmesi;
    int insan_savunma_ustaligi;
    int insan_kusatma_ustaligi;
    int kahraman_sayisi; // Kahraman sayısı
    int canavar_sayisi;  // Canavar sayısı
} InsanBirlikleri;
typedef struct {
    int ork_dovusculeri;
    int mizrakcilar;
    int varg_binicileri;
    int troller;
    char kahramanlar[MAX_KAHRAMAN][30]; 
    char canavarlar[MAX_CANAVAR][30];   
    int elit_egitim;
    int saldiri_gelistirmesi;
    int savunma_ustaligi;
    int kusatma_ustaligi;
    int kahraman_sayisi; 
    int canavar_sayisi;  
} OrkBirlikleri;
typedef struct {
float piyade_saldirigucu;
float okcu_saldirigucu;
float suvari_saldirigucu;
float kusatmaMakineleri_saldirigucu;
float ork_dovusculeri_saldirigucu;
float mizrakcilar_saldirigucu;
float varg_binicileri_saldirigucu;
float troller_saldirigucu;
}saldirigucu;
typedef struct {
float piyade_savunmagucu;
float okcu_savunmagucu;
float suvari_savunmagucu;
float kusatmaMakineleri_savunmagucu;
float ork_dovusculeri_savunmagucu;
float mizrakcilar_savunmagucu;
float varg_binicileri_savunmagucu;
float troller_savunmagucu;
}savunmaGucu;
typedef struct {
    char birim_ismi[50];
    float net_hasar;
    float toplam_insan_saldiri_gucu;
    float toplam_ork_saldiri_gucu;
    float toplam_ork_savunma_gucu;
    float toplam_insan_savunma_gucu;
    float toplam_insan_net_hasar;
    float toplam_ork_net_hasar;
    float piyade_net_hasar;
    float okcu_net_hasar;
    float suvari_net_hasar;
    float kusatmaMakineleri_net_hasar;
    float ork_dovusculeri_net_hasar;
    float varg_binicileri_net_hasar;
    float mizrakcilar_net_hasar;
    float troller_net_hasar;
} NetHasar;
struct MemoryStruct {
    char *memory;
    size_t size;
};
void gucDusur(Birim *birimler, int birim_sayisi);
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0; 
    return realsize;
}
SDL_Texture* loadTexture(const char* file, SDL_Renderer* renderer) {
    SDL_Surface* tempSurface = IMG_Load(file);
    if (!tempSurface) {
        printf("Unable to load image %s! SDL Error: %s\n", file, SDL_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    return texture;
}
void jsonOku(const char *url, InsanBirlikleri *insanlar, OrkBirlikleri *orklar) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1); 
    chunk.size = 0; // Başlangıç boyutu 0

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        // İsteği yap
        res = curl_easy_perform(curl);

        // Hata kontrolü
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() hata: %s\n", curl_easy_strerror(res));
        } else {
            // JSON verisini işleme
            char *satir = strtok(chunk.memory, "\n");
            int insan_bayragi = 0; 
            int ork_bayragi = 0;  
            while (satir != NULL) {
                if (strstr(satir, "\"insan_imparatorlugu\"")) {
                    insan_bayragi = 1;
                    ork_bayragi = 0;
                } else if (strstr(satir, "\"ork_legi\"")) {
                    ork_bayragi = 1;
                    insan_bayragi = 0;
                }
                if (insan_bayragi) {
                    if (strstr(satir, "\"piyadeler\"")) {
                        sscanf(satir, " \"piyadeler\": %d,", &insanlar->piyadeler);
                    } else if (strstr(satir, "\"okcular\"")) {
                        sscanf(satir, " \"okcular\": %d,", &insanlar->okcular);
                    } else if (strstr(satir, "\"suvariler\"")) {
                        sscanf(satir, " \"suvariler\": %d,", &insanlar->suvariler);
                    } else if (strstr(satir, "\"kusatma_makineleri\"")) {
                        sscanf(satir, " \"kusatma_makineleri\": %d,", &insanlar->kusatma_makineleri);
                    } else if (strstr(satir, "\"elit_egitim\"")) {
                        sscanf(satir, " \"elit_egitim\": %d,", &insanlar->insan_elit_egitim);
                    } else if (strstr(satir, "\"saldiri_gelistirmesi\"")) {
                        sscanf(satir, " \"saldiri_gelistirmesi\": %d,", &insanlar->insan_saldiri_gelistirmesi);
                    } else if (strstr(satir, "\"savunma_ustaligi\"")) {
                        sscanf(satir, " \"savunma_ustaligi\": %d,", &insanlar->insan_savunma_ustaligi);
                    } else if (strstr(satir, "\"kusatma_ustaligi\"")) {
                        sscanf(satir, " \"kusatma_ustaligi\": %d,", &insanlar->insan_kusatma_ustaligi);
                    }
                    for (int i = 0; i < 5; i++) {
                        if (strstr(satir, insan_kahramanlar[i])) {
                            strcpy(insanlar->kahramanlar[insanlar->kahraman_sayisi++], insan_kahramanlar[i]);
                        }
                    }
                    for (int i = 0; i < 5; i++) {
                        if (strstr(satir, insan_canavarlar[i])) {
                            strcpy(insanlar->canavarlar[insanlar->canavar_sayisi++], insan_canavarlar[i]);
                        }
                    }
                }
                if (ork_bayragi) {
                    if (strstr(satir, "\"ork_dovusculeri\"")) {
                        sscanf(satir, " \"ork_dovusculeri\": %d,", &orklar->ork_dovusculeri);
                    } else if (strstr(satir, "\"mizrakcilar\"")) {
                        sscanf(satir, " \"mizrakcilar\": %d,", &orklar->mizrakcilar);
                    } else if (strstr(satir, "\"varg_binicileri\"")) {
                        sscanf(satir, " \"varg_binicileri\": %d,", &orklar->varg_binicileri);
                    } else if (strstr(satir, "\"troller\"")) {
                        sscanf(satir, " \"troller\": %d,", &orklar->troller);
                    } else if (strstr(satir, "\"elit_egitim\"")) {
                        sscanf(satir, " \"elit_egitim\": %d,", &orklar->elit_egitim);
                    } else if (strstr(satir, "\"saldiri_gelistirmesi\"")) {
                        sscanf(satir, " \"saldiri_gelistirmesi\": %d,", &orklar->saldiri_gelistirmesi);
                    } else if (strstr(satir, "\"savunma_ustaligi\"")) {
                        sscanf(satir, " \"savunma_ustaligi\": %d,", &orklar->savunma_ustaligi);
                    } else if (strstr(satir, "\"kusatma_ustaligi\"")) {
                        sscanf(satir, " \"kusatma_ustaligi\": %d,", &orklar->kusatma_ustaligi);
                    }
                    for (int i = 0; i < 5; i++) {
                        if (strstr(satir, ork_kahramanlar[i])) {
                            strcpy(orklar->kahramanlar[orklar->kahraman_sayisi++], ork_kahramanlar[i]);
                        }
                    }
                    for (int i = 0; i < 5; i++) {
                        if (strstr(satir, ork_canavarlar[i])) {
                            strcpy(orklar->canavarlar[orklar->canavar_sayisi++], ork_canavarlar[i]);
                        }
                    }
                }

                // Sonraki satıra geç
                satir = strtok(NULL, "\n");
            }
        }
        free(chunk.memory);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
}
char* dosyaOku(const char* dosyaAdi) {
    FILE *fp = fopen(dosyaAdi, "r");
    if (fp == NULL) {
        printf("Dosya Acılamadı.\n");
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
void json_verilerini_isle_canavar(const char* dosya_yolu, Irk* insan_irki, Irk* ork_lejyonu) {
    FILE* dosya = fopen(dosya_yolu, "r");
    if (dosya == NULL) {
        printf("Dosya açılamadı: %s\n", dosya_yolu);
        return;
    }

    char satir[256];
    insan_irki->birim_sayisi = 0;
    ork_lejyonu->birim_sayisi = 0;
    int insan_okunma = 0, ork_okunma = 0;

    while (fgets(satir, sizeof(satir), dosya)) {
        if (strstr(satir, "\"insan_imparatorlugu\":") != NULL) {
            insan_okunma = 1;
            ork_okunma = 0;
            continue;
        }
        if (strstr(satir, "\"ork_legi\":") != NULL) {
            ork_okunma = 1;
            insan_okunma = 0;
            continue;
        }
        if (insan_okunma) {
            if (strstr(satir, "\"etki_degeri\"") != NULL) {
                sscanf(satir, " \"etki_degeri\": \"%d\",", &insan_irki->birimler[insan_irki->birim_sayisi].etki_degeri);
            } else if (strstr(satir, "\"etki_turu\"") != NULL) {
                sscanf(satir, " \"etki_turu\": \"%[^\"]\",", insan_irki->birimler[insan_irki->birim_sayisi].etki_turu);
            } else if (strstr(satir, "\"aciklama\"") != NULL) {
                char aciklama[256];
                sscanf(satir, " \"aciklama\": \"%[^\"]\",", aciklama);
                etkiledigi_birim_ayikla(aciklama, insan_irki->birimler[insan_irki->birim_sayisi].etkiledigi_birim);
            } else if (strstr(satir, "}") != NULL) {
                insan_irki->birim_sayisi++;
            } else if (strstr(satir, "\"") != NULL) {
                sscanf(satir, " \"%[^\"]\": {", insan_irki->birimler[insan_irki->birim_sayisi].isim);
            }
        }
        if (ork_okunma) {
            if (strstr(satir, "\"etki_degeri\"") != NULL) {
                sscanf(satir, " \"etki_degeri\": \"%d\",", &ork_lejyonu->birimler[ork_lejyonu->birim_sayisi].etki_degeri);
            } else if (strstr(satir, "\"etki_turu\"") != NULL) {
                sscanf(satir, " \"etki_turu\": \"%[^\"]\",", ork_lejyonu->birimler[ork_lejyonu->birim_sayisi].etki_turu);
            } else if (strstr(satir, "\"aciklama\"") != NULL) {
                char aciklama[256];
                sscanf(satir, " \"aciklama\": \"%[^\"]\",", aciklama);
                etkiledigi_birim_ayikla(aciklama, ork_lejyonu->birimler[ork_lejyonu->birim_sayisi].etkiledigi_birim);
            } else if (strstr(satir, "}") != NULL) {
                ork_lejyonu->birim_sayisi++;
            } else if (strstr(satir, "\"") != NULL) {
                sscanf(satir, " \"%[^\"]\": {", ork_lejyonu->birimler[ork_lejyonu->birim_sayisi].isim);
            }
        }
    }

    fclose(dosya);
}
void jsonAyristirVeAta(const char* json, InsanIrki* insanIrki, OrkLegi* orkLegi) {
    char *jsonKopya = strdup(json);
    char *tok = strtok(jsonKopya, "{},:\"[]");

    int insanIndex = 0, orkIndex = 0;

    while (tok != NULL) {
        if (strcmp(tok, "piyadeler") == 0 || strcmp(tok, "okcular") == 0 || strcmp(tok, "suvariler") == 0 || strcmp(tok, "kusatma_makineleri") == 0) {
            strcpy(insanIrki->birimler[insanIndex].isim, tok);
            tok = strtok(NULL, "{}:,\"[]");
            while (tok != NULL && strcmp(tok, "saldiri") != 0) tok = strtok(NULL, "{}:,\"[]");
            insanIrki->birimler[insanIndex].saldiri = atoi(strtok(NULL, "{}:,\"[]"));

            while (tok != NULL && strcmp(tok, "savunma") != 0) tok = strtok(NULL, "{}:,\"[]");
            insanIrki->birimler[insanIndex].savunma = atoi(strtok(NULL, "{}:,\"[]"));

            while (tok != NULL && strcmp(tok, "saglik") != 0) tok = strtok(NULL, "{}:,\"[]");
            insanIrki->birimler[insanIndex].saglik = atoi(strtok(NULL, "{}:,\"[]"));

            // Max sağlık bilgisini saglik ile aynı yerden çekiyoruz
            insanIrki->birimler[insanIndex].max_saglik = insanIrki->birimler[insanIndex].saglik;

            while (tok != NULL && strcmp(tok, "kritik_sans") != 0) tok = strtok(NULL, "{}:,\"[]");
            insanIrki->birimler[insanIndex].kritik_sans = atoi(strtok(NULL, "{}:,\"[]"));

            insanIndex++;
        }
        else if (strcmp(tok, "ork_dovusculeri") == 0 || strcmp(tok, "mizrakcilar") == 0 || strcmp(tok, "varg_binicileri") == 0 || strcmp(tok, "troller") == 0) {
            strcpy(orkLegi->birimler[orkIndex].isim, tok);
            tok = strtok(NULL, "{}:,\"[]");
            while (tok != NULL && strcmp(tok, "saldiri") != 0) tok = strtok(NULL, "{}:,\"[]");
            orkLegi->birimler[orkIndex].saldiri = atoi(strtok(NULL, "{}:,\"[]"));

            while (tok != NULL && strcmp(tok, "savunma") != 0) tok = strtok(NULL, "{}:,\"[]");
            orkLegi->birimler[orkIndex].savunma = atoi(strtok(NULL, "{}:,\"[]"));

            while (tok != NULL && strcmp(tok, "saglik") != 0) tok = strtok(NULL, "{}:,\"[]");
            orkLegi->birimler[orkIndex].saglik = atoi(strtok(NULL, "{}:,\"[]"));

            // Max sağlık bilgisini saglik ile aynı yerden çekiyoruz
            orkLegi->birimler[orkIndex].max_saglik = orkLegi->birimler[orkIndex].saglik;

            while (tok != NULL && strcmp(tok, "kritik_sans") != 0) tok = strtok(NULL, "{}:,\"[]");
            orkLegi->birimler[orkIndex].kritik_sans = atoi(strtok(NULL, "{}:,\"[]"));

            orkIndex++;
        }
        tok = strtok(NULL, "{}:,\"[]");
    }

    insanIrki->birim_sayisi = insanIndex;
    orkLegi->birim_sayisi = orkIndex;
    free(jsonKopya);
}
void aciklamadan_birimi_cek(char *aciklama, char *birim) {
    if (strstr(aciklama, "piyadelere")) {
        strcpy(birim, "piyadeler");
    } else if (strstr(aciklama, "okcularin")) {
        strcpy(birim, "okcular");
    } else if (strstr(aciklama, "suvarilere")) {
        strcpy(birim, "suvariler");
    } else if (strstr(aciklama, "kusatma_makinelerinin")) {
        strcpy(birim, "kusatma_makineleri");
    } else if (strstr(aciklama, "ork_dovusculerine")) {
        strcpy(birim, "ork_dovusculeri");
    } else if (strstr(aciklama, "varg_binicilerine")) {
        strcpy(birim, "varg_binicileri");
    } else if (strstr(aciklama, "trollerin")) {
        strcpy(birim, "troller");
    } else if (strstr(aciklama, "tum_birimlere")) {
        strcpy(birim, "tum_birimler");
    } else {
        strcpy(birim, "bilinmiyor");
    }
}
void etkiledigi_birim_ayikla(const char* aciklama, char* etkiledigi_birim) {//canavar
    if (strstr(aciklama, "piyadeler")) {
        strcpy(etkiledigi_birim, "piyadeler");
    } else if (strstr(aciklama, "okcular")) {
        strcpy(etkiledigi_birim, "okcular");
    } else if (strstr(aciklama, "suvariler")) {
        strcpy(etkiledigi_birim, "suvariler");
    } else if (strstr(aciklama, "troller")) {
        strcpy(etkiledigi_birim, "troller");
    } else if (strstr(aciklama, "Troller")) {
        strcpy(etkiledigi_birim, "troller");
    } else if (strstr(aciklama, "varg_binicileri")) {
        strcpy(etkiledigi_birim, "varg_binicileri");
    } else if (strstr(aciklama, "ork_dovusculeri")) {
        strcpy(etkiledigi_birim, "ork_dovusculeri");
    } else if (strstr(aciklama, "mizrakcilar")) {
        strcpy(etkiledigi_birim, "mizrakcilar");
    } else {
        strcpy(etkiledigi_birim, "Bilinmiyor");
    }
}
char* grupIcindenDegerBul(const char* grup, const char* kahraman, const char* anahtar) {
    char* kahramanPtr = strstr(grup, kahraman);
    if (!kahramanPtr) {
        return NULL;
    }
    char anahtarTam[100];
    sprintf(anahtarTam, "\"%s\":", anahtar);
    char* ptr = strstr(kahramanPtr, anahtarTam);
    if (ptr) {
        ptr = strchr(ptr, ':'); 
        if (ptr) {
            ptr++;  // ":" işaretinden sonrasına git
            while (*ptr == ' ' || *ptr == '"') ptr++;  
            char* end = strchr(ptr, '"');  
            if (end) {
                size_t uzunluk = end - ptr;
                char* deger = (char*)malloc(uzunluk + 1);
                if (deger != NULL) {
                    strncpy(deger, ptr, uzunluk);
                    deger[uzunluk] = '\0';
                    return deger;
                } else {
                    printf("Bellek ayrilamadi.\n");
                }
            }
        }
    }
    return NULL;
}
void json_verilerini_isle(char* json, InsanKahramanlari* insanKahramanlari, OrkKahramanlari* orkKahramanlari) {
    const char* insan_kahramanlar[] = {"Alparslan", "Fatih_Sultan_Mehmet", "Mete_Han", "Yavuz_Sultan_Selim", "Tugrul_Bey"};
    const char* ork_kahramanlar[] = {"Goruk_Vahsi", "Thruk_Kemikkiran", "Vrog_Kafakiran", "Ugar_Zalim"};
    char* insan_imparatorlugu = strstr(json, "insan_imparatorlugu");
    char* ork_legi = strstr(json, "ork_legi");

    if (insan_imparatorlugu == NULL || ork_legi == NULL) {
        printf("JSON'da insan_imparatorlugu veya ork_legi bulunamadı.\n");
        return;
    }
    for (int i = 0; i < 5; i++) {
        // Kahraman ismini al
        strcpy(insanKahramanlari->kahramanlar[i].isim, insan_kahramanlar[i]);

        char* bonus_turu = grupIcindenDegerBul(insan_imparatorlugu, insan_kahramanlar[i], "bonus_turu");
        if (bonus_turu != NULL) {
            strcpy(insanKahramanlari->kahramanlar[i].bonus_turu, bonus_turu);
            free(bonus_turu);
        } else {
            printf("%s kahramanı için bonus_turu bulunamadı.\n", insan_kahramanlar[i]);
            continue;
        }

        char* bonus_degeri_str = grupIcindenDegerBul(insan_imparatorlugu, insan_kahramanlar[i], "bonus_degeri");
        if (bonus_degeri_str != NULL) {
            insanKahramanlari->kahramanlar[i].bonus_degeri = atoi(bonus_degeri_str);
            free(bonus_degeri_str);
        } else {
            printf("%s kahramanı için bonus_degeri bulunamadı.\n", insan_kahramanlar[i]);
            continue;
        }

        char* aciklama = grupIcindenDegerBul(insan_imparatorlugu, insan_kahramanlar[i], "aciklama");
        if (aciklama != NULL) {
            strcpy(insanKahramanlari->kahramanlar[i].aciklama, aciklama);
            aciklamadan_birimi_cek(insanKahramanlari->kahramanlar[i].aciklama, insanKahramanlari->kahramanlar[i].etkiledigi_birim);
            free(aciklama);
        } else {
            printf("%s kahramanı için aciklama bulunamadı.\n", insan_kahramanlar[i]);
        }

        insanKahramanlari->kahraman_sayisi++;
    }
    for (int i = 0; i < 4; i++) {
        // Kahraman ismini al
        strcpy(orkKahramanlari->kahramanlar[i].isim, ork_kahramanlar[i]);

        char* bonus_turu = grupIcindenDegerBul(ork_legi, ork_kahramanlar[i], "bonus_turu");
        if (bonus_turu != NULL) {
            strcpy(orkKahramanlari->kahramanlar[i].bonus_turu, bonus_turu);
            free(bonus_turu);
        } else {
            printf("%s kahramanı için bonus_turu bulunamadı.\n", ork_kahramanlar[i]);
            continue;
        }

        char* bonus_degeri_str = grupIcindenDegerBul(ork_legi, ork_kahramanlar[i], "bonus_degeri");
        if (bonus_degeri_str != NULL) {
            orkKahramanlari->kahramanlar[i].bonus_degeri = atoi(bonus_degeri_str);
            free(bonus_degeri_str);
        } else {
            printf("%s kahramanı için bonus_degeri bulunamadı.\n", ork_kahramanlar[i]);
            continue;
        }

        char* aciklama = grupIcindenDegerBul(ork_legi, ork_kahramanlar[i], "aciklama");
        if (aciklama != NULL) {
            strcpy(orkKahramanlari->kahramanlar[i].aciklama, aciklama);
            aciklamadan_birimi_cek(orkKahramanlari->kahramanlar[i].aciklama, orkKahramanlari->kahramanlar[i].etkiledigi_birim);
            free(aciklama);
        } else {
            printf("%s kahramanı için aciklama bulunamadı.\n", ork_kahramanlar[i]);
        }
        orkKahramanlari->kahraman_sayisi++;
    }}
char* degerBulArastirmaNiteligi(const char* json, const char* anahtar, const char* alt_json) {
    char* ptr = strstr(alt_json, anahtar);
    if (ptr) {
        ptr = strchr(ptr, ':');  // ":" isaretini bul
        if (ptr) {
            ptr++;  // ":" isaretinden sonrasina git
            while (*ptr == ' ' || *ptr == '"') ptr++; 
            char* end = strchr(ptr, '"');
            if (end) {
                size_t uzunluk = end - ptr;
                char* deger = (char*)malloc(uzunluk + 1);
                strncpy(deger, ptr, uzunluk);
                deger[uzunluk] = '\0';
                return deger;
            }
        }
    }
    return NULL;
}
char* arastirmaJsonBul(const char* json, const char* arastirma_adi, const char* seviye) {
    char* ptr = strstr(json, arastirma_adi);
    if (ptr) {
        ptr = strstr(ptr, seviye);
        if (ptr) {
            char* start = ptr;
            char* end = strchr(ptr, '}');
            if (end) {
                size_t uzunluk = end - start + 1;
                char* arastirma_json = (char*)malloc(uzunluk + 1);
                strncpy(arastirma_json, start, uzunluk);
                arastirma_json[uzunluk] = '\0';
                return arastirma_json;
            }
        }
    }
    return NULL;
}
void arastirmalariAyrisir(const char* json, const char* arastirmalar[], int arastirma_sayisi, Arastirma* arastirmalar_struct, int* arastirma_sayisi_ptr) {
    int index = 0;

    for (int i = 0; i < arastirma_sayisi; i++) {
        for (int seviye = 1; seviye <= 3; seviye++) {
            char seviye_str[20];
            sprintf(seviye_str, "seviye_%d", seviye);

            char* arastirma_json = arastirmaJsonBul(json, arastirmalar[i], seviye_str);
            if (arastirma_json != NULL) {
                strcpy(arastirmalar_struct[index].isim, arastirmalar[i]);
                arastirmalar_struct[index].seviye = seviye;

                // Degerini al
                char* deger = degerBulArastirmaNiteligi(json, "deger", arastirma_json);
                if (deger != NULL) {
                    arastirmalar_struct[index].deger = atoi(deger);
                    free(deger);
                }

                // Aciklamayi al
                char* aciklama = degerBulArastirmaNiteligi(json, "aciklama", arastirma_json);
                if (aciklama != NULL) {
                    strcpy(arastirmalar_struct[index].aciklama, aciklama);
                    free(aciklama);
                }

                free(arastirma_json);
                index++;
            }
        }
    }

    *arastirma_sayisi_ptr = index;
}
void extract_field(char *source, const char *field_name, char *destination) {
    char *start = strstr(source, field_name);
    if (start) {
        start += strlen(field_name) + 4;  // "field_name": "
        char *end = strchr(start, '"');
        if (end) {
            strncpy(destination, start, end - start);
            destination[end - start] = '\0';
        }
    }
}
void extract_int_field(char *source, const char *field_name, int *destination) {
    char *start = strstr(source, field_name);
    if (start) {
        start += strlen(field_name) + 3;  // "field_name": (sayýsal veri olduðu için sadece 3 atla)
        *destination = strtol(start, NULL, 10);  // strtol ile daha saðlam sayýsal ayrýþtýrma
        if (*destination == 0) {
            char *num_start = start;
            while (*num_start && !isdigit(*num_start)) {
                num_start++;
            }
            *destination = strtol(num_start, NULL, 10);  // Eðer baþta bulamadýysak sayýyý tekrar ara
        }
    }
}
void verileriYazdir(const InsanBirlikleri *insanlar, const OrkBirlikleri *orklar) {
    printf("İnsan İmparatorluğu Birlikleri:\n");
    printf("Piyadeler: %d\n", insanlar->piyadeler);
    printf("Okçular: %d\n", insanlar->okcular);
    printf("Süvariler: %d\n", insanlar->suvariler);
    printf("Kuşatma Makineleri: %d\n", insanlar->kusatma_makineleri);
    printf("Kahramanlar:%d \n",insanlar->kahraman_sayisi);

    printf("Kahramanlar: ");
    for (int i = 0; i < insanlar->kahraman_sayisi; i++) {
        printf("%s", insanlar->kahramanlar[i]);
        if (i < insanlar->kahraman_sayisi - 1) {
            printf(", ");
        }
    }
    printf("\n");

    printf("Canavarlar: ");
    for (int i = 0; i < insanlar->canavar_sayisi; i++) {
        printf("%s", insanlar->canavarlar[i]);
        if (i < insanlar->canavar_sayisi - 1) {
            printf(", ");
        }
    }
    printf("\n");

    printf("Elit Eğitim Seviyesi: %d\n", insanlar->insan_elit_egitim);
    printf("Saldırı Geliştirmesi Seviyesi: %d\n", insanlar->insan_saldiri_gelistirmesi);
    printf("Savunma Ustalığı: %d\n", insanlar->insan_savunma_ustaligi);
    printf("Kuşatma Ustalığı: %d\n", insanlar->insan_kusatma_ustaligi);

    printf("\nOrk Lejyonu Birlikleri:\n");
    printf("Ork Dövüşçüleri: %d\n", orklar->ork_dovusculeri);
    printf("Mızrakçılar: %d\n", orklar->mizrakcilar);
    printf("Varg Binicileri: %d\n", orklar->varg_binicileri);
    printf("Troller: %d\n", orklar->troller);

    printf("Kahramanlar: ");
    for (int i = 0; i < orklar->kahraman_sayisi; i++) {
        printf("%s", orklar->kahramanlar[i]);
        if (i < orklar->kahraman_sayisi - 1) {
            printf(", ");
        }
    }
    printf("\n");

    printf("Canavarlar: ");
    for (int i = 0; i < orklar->canavar_sayisi; i++) {
        printf("%s", orklar->canavarlar[i]);
        if (i < orklar->canavar_sayisi - 1) {
            printf(", ");
        }
    }
    printf("\n");

    printf("Elit Eğitim Seviyesi: %d\n", orklar->elit_egitim);
    printf("saldiri_gelistirmesi Seviyesi: %d\n", orklar->saldiri_gelistirmesi);
    printf("Savunma Ustalığı: %d\n", orklar->savunma_ustaligi);
    printf("Kuşatma Ustalığı: %d\n", orklar->kusatma_ustaligi);
}
void kahramanlarin_etkisini_uygula(InsanBirlikleri insanlar, OrkBirlikleri orklar, Birim *insan_birimler, int insan_birim_sayisi, Birim *ork_birimler, int ork_birim_sayisi, InsanKahramanlari *insanKahramanlari, OrkKahramanlari *orkKahramanlari) {
    // İnsan kahramanlarının etkisini uygula
     printf("Kahraman Etkileri Uygulaniyor...\n");
    for (int j = 0; j < insanlar.kahraman_sayisi; j++) {
        const char* kahraman_ismi = insanlar.kahramanlar[j];
        for (int k = 0; k < insanKahramanlari->kahraman_sayisi; k++) {
            if (strcmp(kahraman_ismi, insanKahramanlari->kahramanlar[k].isim) == 0) {
                const char* etkiledigi_birim = insanKahramanlari->kahramanlar[k].etkiledigi_birim;
                int bonus_degeri = insanKahramanlari->kahramanlar[k].bonus_degeri;
                const char* bonus_turu = insanKahramanlari->kahramanlar[k].bonus_turu;

                for (int i = 0; i < insan_birim_sayisi; i++) {
                    if (strcmp(insan_birimler[i].isim, etkiledigi_birim) == 0) {
                        if (strcmp(bonus_turu, "saldiri") == 0) {
                            insan_birimler[i].saldiri += insan_birimler[i].saldiri*bonus_degeri/100;
                            printf("%s biriminin sadliri gucu %d arttirildi. Yeni saldiri gucu: %d\n",
                                   insan_birimler[i].isim, bonus_degeri, insan_birimler[i].saldiri);
                        } else if (strcmp(bonus_turu, "savunma") == 0) {
                            insan_birimler[i].savunma +=insan_birimler[i].savunma*bonus_degeri/100;
                            printf("%s biriminin savunma gucu %d arttirildi. Yeni savunma gucu: %d\n",
                                   insan_birimler[i].isim, bonus_degeri, insan_birimler[i].savunma);
                        }
                        else if (strcmp(bonus_turu, "kritik_sans") == 0) {
                            insan_birimler[i].kritik_sans += insan_birimler[i].kritik_sans*bonus_degeri/100;
                            printf("%s biriminin kritik sansi yuzde %d arttirildi. Yeni kritik sansi: %d\n",
                                   insan_birimler[i].isim, bonus_degeri, insan_birimler[i].kritik_sans);
                        }


                    }
                }
            }
        }
    }
    for (int j = 0; j < orklar.kahraman_sayisi; j++) {
        const char* kahraman_ismi = orklar.kahramanlar[j];
        for (int k = 0; k < orkKahramanlari->kahraman_sayisi; k++) {
            if (strcmp(kahraman_ismi, orkKahramanlari->kahramanlar[k].isim) == 0) {
                const char* etkiledigi_birim = orkKahramanlari->kahramanlar[k].etkiledigi_birim;
                int bonus_degeri = orkKahramanlari->kahramanlar[k].bonus_degeri;
                const char* bonus_turu = orkKahramanlari->kahramanlar[k].bonus_turu;
                for (int i = 0; i < ork_birim_sayisi; i++) {

                    if (strcmp(ork_birimler[i].isim,etkiledigi_birim) == 0) {
                        if (strcmp(bonus_turu, "saldiri") == 0) {
                            ork_birimler[i].saldiri += ork_birimler[i].saldiri*bonus_degeri/100;
                            printf("%s biriminin saldiri gucu %d arttirildi. Yeni saldiri gucu: %d\n",
                                   ork_birimler[i].isim, bonus_degeri, ork_birimler[i].saldiri);
                        } else if (strcmp(bonus_turu, "savunma") == 0) {
                            ork_birimler[i].savunma += ork_birimler[i].savunma*bonus_degeri/100;
                            printf("%s biriminin savunma gucu %d arttirildi. Yeni savunma gücü: %d\n",
                                   ork_birimler[i].isim, bonus_degeri, ork_birimler[i].savunma);
                        }
                        else if (strcmp(bonus_turu, "kritik_sans") == 0) {
                            ork_birimler[i].kritik_sans += ork_birimler[i].kritik_sans*bonus_degeri/100;
                            printf("%s biriminin kritik sansi %d arttirildi. Yeni kritik sansi: %d\n",
                                   ork_birimler[i].isim, bonus_degeri, ork_birimler[i].kritik_sans);
                        }
                    }
                    else if (strcmp(etkiledigi_birim, "tum_birimler") == 0) {
                                for (int i = 0; i < ork_birim_sayisi; i++) {ork_birimler[i].savunma += ork_birimler[i].savunma*bonus_degeri/100;}
                            printf("Tum birimlerin savunma gucu yuzde %d arttirildi. \n",bonus_degeri);
                            break;
                        }
                }
            }
        }
    }
}
void canavarlarin_etkisini_uygula(InsanBirlikleri insanlar, OrkBirlikleri orklar, Birim *insan_birimler, int insan_birim_sayisi, Birim *ork_birimler, int ork_birim_sayisi, Irk *insanCanavarlari, Irk *orkCanavarlari) {
    printf("Canavar Etkileri Uygulaniyor...\n");
    for (int j = 0; j < insanlar.canavar_sayisi; j++) {
        const char* canavar_ismi = insanlar.canavarlar[j];  
        for (int k = 0; k < insanCanavarlari->birim_sayisi; k++) {
            if (strcmp(canavar_ismi, insanCanavarlari->birimler[k].isim) == 0) {
                const char* etkiledigi_birim = insanCanavarlari->birimler[k].etkiledigi_birim;
                int etki_degeri = insanCanavarlari->birimler[k].etki_degeri;
                const char* etki_turu = insanCanavarlari->birimler[k].etki_turu;
                for (int i = 0; i < insan_birim_sayisi; i++) {
                    if (strcmp(insan_birimler[i].isim, etkiledigi_birim) == 0) {
                        if (strcmp(etki_turu, "saldiri") == 0) {
                            insan_birimler[i].saldiri += insan_birimler[i].saldiri * etki_degeri / 100;
                            printf("%s biriminin saldırı gücü %d%% arttırıldı. Yeni saldırı gücü: %d\n",
                                   insan_birimler[i].isim, etki_degeri, insan_birimler[i].saldiri);
                        } else if (strcmp(etki_turu, "savunma") == 0) {
                            insan_birimler[i].savunma += insan_birimler[i].savunma * etki_degeri / 100;
                            printf("%s biriminin savunma gücü %d%% arttırıldı. Yeni savunma gücü: %d\n",
                                   insan_birimler[i].isim, etki_degeri, insan_birimler[i].savunma);
                        }
                        else if (strcmp(etki_turu, "kritik_sans") == 0) {
                            insan_birimler[i].kritik_sans += insan_birimler[i].kritik_sans * etki_degeri / 100;
                            printf("%s biriminin kritik şansı %d%% arttırıldı. Yeni kritik şansı: %d\n",
                                   insan_birimler[i].isim, etki_degeri, insan_birimler[i].kritik_sans);
                        }
                    }
                }
            }
        }
    }
    for (int j = 0; j < orklar.canavar_sayisi; j++) {
        const char* canavar_ismi = orklar.canavarlar[j];  
        for (int k = 0; k < orkCanavarlari->birim_sayisi; k++) {
            if (strcmp(canavar_ismi, orkCanavarlari->birimler[k].isim) == 0) {
                const char* etkiledigi_birim = orkCanavarlari->birimler[k].etkiledigi_birim;
                int etki_degeri = orkCanavarlari->birimler[k].etki_degeri;
                const char* etki_turu = orkCanavarlari->birimler[k].etki_turu;

                for (int i = 0; i < ork_birim_sayisi; i++) {
                    if (strcmp(ork_birimler[i].isim, etkiledigi_birim) == 0) {
                        if (strcmp(etki_turu, "saldiri") == 0) {
                            ork_birimler[i].saldiri += ork_birimler[i].saldiri * etki_degeri / 100;
                            printf("%s biriminin saldırı gücü %d arttırıldı. Yeni saldırı gücü: %d\n",
                                   ork_birimler[i].isim, etki_degeri, ork_birimler[i].saldiri);
                        } else if (strcmp(etki_turu, "savunma") == 0) {
                            ork_birimler[i].savunma += ork_birimler[i].savunma * etki_degeri / 100;
                            printf("%s biriminin savunma gücü %d arttırıldı. Yeni savunma gücü: %d\n",
                                   ork_birimler[i].isim, etki_degeri, ork_birimler[i].savunma);
                        }
                        else if (strcmp(etki_turu, "kritik_sans") == 0) {
                            ork_birimler[i].kritik_sans += ork_birimler[i].kritik_sans * etki_degeri / 100;
                            printf("%s biriminin kritik şansı %d arttırıldı. Yeni kritik şansı: %d\n",
                                   ork_birimler[i].isim, etki_degeri, ork_birimler[i].kritik_sans);
                        }
                    }
                    else if (strcmp(etkiledigi_birim, "tum_birimler") == 0) {
                        for (int i = 0; i < ork_birim_sayisi; i++) {
                            ork_birimler[i].savunma += ork_birimler[i].savunma * etki_degeri / 100;
                        }
                        printf("Tüm ork birimlerinin savunma gücü %d arttırıldı.\n", etki_degeri);
                        break;
                    }
                }
            }
        }
    }
}
void arastirma_niteliklerini_uygula(InsanBirlikleri insanlar, OrkBirlikleri orklar, Birim *insan_birimler, int insan_birim_sayisi, Birim *ork_birimler, int ork_birim_sayisi, ArastirmaListesi *arastirmaListesi) {
    for (int j = 0; j < arastirmaListesi->arastirma_sayisi; j++) {
        const char* arastirma_ismi = arastirmaListesi->arastirmalar[j].isim;
        int arastirma_seviyesi = arastirmaListesi->arastirmalar[j].seviye;
        int arastirma_degeri = arastirmaListesi->arastirmalar[j].deger;
        if (arastirma_seviyesi > 0) {
            // İnsan araştırmalarının etkisini uygula
            if (strcmp(arastirma_ismi, "savunma_ustaligi") == 0 && insanlar.insan_savunma_ustaligi==arastirma_seviyesi) {
                printf("Insan savunma ustaligi arastirmasi bulundu ve uygulaniyor.\n");
                for (int i = 0; i < insan_birim_sayisi; i++) {
                    insan_birimler[i].savunma += insan_birimler[i].savunma * arastirma_degeri / 100;
                 }   printf("Tüm insan Birimlerinin savunma gucu yuzde %d arttirildi.\n\n",arastirma_degeri);

            } else if (strcmp(arastirma_ismi, "saldiri_gelistirmesi") == 0 && insanlar.insan_saldiri_gelistirmesi ==arastirma_seviyesi) {
                printf("Insan saldırı gelistirmesi arastirmasi bulundu ve uygulaniyor.\n");
                for (int i = 0; i < insan_birim_sayisi; i++) {
                    insan_birimler[i].saldiri += insan_birimler[i].saldiri * arastirma_degeri / 100;
                 }   printf("Tüm insan Birimlerinin saldiri gucu yuzde %d arttirildi.\n\n",arastirma_degeri);

            } else if (strcmp(arastirma_ismi, "kusatma_ustaligi") == 0 && insanlar.insan_kusatma_ustaligi ==arastirma_seviyesi) {
                printf("Insan kusatma ustaligi arastirmasi bulundu ve uygulaniyor.\n");
                for (int i = 0; i < insan_birim_sayisi; i++) {
                    if (strcmp(insan_birimler[i].isim, "kusatma_makineleri") == 0) {
                        insan_birimler[i].saldiri += insan_birimler[i].saldiri * arastirma_degeri / 100;}
                }printf("Insan kusatma makinelerinin saldiri gucu yuzde %d arttirildi.\n\n", arastirma_degeri);
            }
            if (strcmp(arastirma_ismi, "savunma_ustaligi") == 0 && orklar.savunma_ustaligi ==arastirma_seviyesi) {
                 printf("Ork savunma ustaligi arastirmasi bulundu ve uygulaniyor.\n");
                for (int i = 0; i < ork_birim_sayisi; i++) {
                    ork_birimler[i].savunma += ork_birimler[i].savunma * arastirma_degeri / 100;
                }    printf("Tüm Ork Birimlerinin savunma gucu  yuzde %d arttirildi.\n\n", arastirma_degeri);

            } else if (strcmp(arastirma_ismi, "saldiri_gelistirmesi") == 0 && orklar.saldiri_gelistirmesi ==arastirma_seviyesi) {
                 printf("Ork saldiri gelistirmesi arastirmasi bulundu ve uygulaniyor.\n");
                for (int i = 0; i < ork_birim_sayisi; i++) {
                    ork_birimler[i].saldiri += ork_birimler[i].saldiri * arastirma_degeri / 100;
                 }   printf("Tum Ork birimlerinin saldiri gucu yuzde %d arttirildi. \n\n",arastirma_degeri);

            } else if (strcmp(arastirma_ismi, "kusatma_ustaligi") == 0 && orklar.kusatma_ustaligi==arastirma_seviyesi) {
                printf("Ork kusatma ustaligi arastirmasi bulundu ve uygulaniyor.\n");
                for (int i = 0; i < ork_birim_sayisi; i++) {
                    if (strcmp(ork_birimler[i].isim, "troller") == 0) {
                        ork_birimler[i].saldiri += ork_birimler[i].saldiri * arastirma_degeri / 100;
                     }   }
                     printf("Ork trollerinin saldiri gucu yuzde %d arttirildi.\n\n", arastirma_degeri);
            }
        }
    }
}
void birimSaldiriSavunmaHesapla(Birim *birim, float birim_sayisi, float *toplam_saldiri, float *toplam_savunma, int sayac,ArastirmaListesi *arastirmaListesi,InsanBirlikleri* insanlar,OrkBirlikleri* orklar) {
   int k=0;
    if (birim_sayisi > 0) {
  if(birim->kritik_sans==0)birim->kritik_sans=-1;
int kritik_vurus_turu = (int)floor(100.0 / birim->kritik_sans);
        if (sayac%kritik_vurus_turu==0) {
        for (int j = 0; j < arastirmaListesi->arastirma_sayisi; j++) {
        const char* arastirma_ismi = arastirmaListesi->arastirmalar[j].isim;
        int arastirma_seviyesi = arastirmaListesi->arastirmalar[j].seviye;
        int arastirma_degeri = arastirmaListesi->arastirmalar[j].deger;
        if (arastirma_seviyesi > 0) {
            if (strcmp(arastirma_ismi, "elit_egitim") == 0 && insanlar->insan_elit_egitim ==arastirma_seviyesi ) {
                printf("Insan elit egitim arastirmasi bulundu ve uygulaniyor.\n");k=1;
                printf("%s birimi bu tur kritik vurus yapıyor\n",birim->isim);
            *toplam_saldiri = (birim->saldiri * birim_sayisi) * 1.5*(1+arastirma_degeri/10);
            *toplam_savunma = birim->savunma * birim_sayisi;
            }
             if (strcmp(arastirma_ismi, "elit_egitim") == 0 && orklar->elit_egitim==arastirma_seviyesi) {
                 printf("Ork elit egitim arastirmasi bulundu ve uygulaniyor.\n");k=1;
                 *toplam_saldiri = (birim->saldiri * birim_sayisi) * 1.5*(1+arastirma_degeri/10);
                *toplam_savunma = birim->savunma * birim_sayisi;
               }}}
              if(k!=0){
             printf("%s birimi bu tur kritik vurus yapıyor\n",birim->isim);
            *toplam_saldiri = (birim->saldiri * birim_sayisi) * 1.5;
            *toplam_savunma = birim->savunma * birim_sayisi;
              }
        } else {
            *toplam_saldiri = birim->saldiri * birim_sayisi;
            *toplam_savunma = birim->savunma * birim_sayisi;
        }
    } else {
        *toplam_saldiri = 0;
        *toplam_savunma = 0;
    }
}

void SaldiriSavunmaHesapla(InsanIrki* insanIrki, saldirigucu* toplamSaldiri, InsanBirlikleri* insanlar, savunmaGucu* toplamSavunma, OrkBirlikleri* orklar, OrkLegi* orkLegi,int sayac,ArastirmaListesi *arastirmaListesi) {
    for (int i = 0; i < insanIrki->birim_sayisi; i++) {
        if (strcmp(insanIrki->birimler[i].isim, "piyadeler") == 0) {
            birimSaldiriSavunmaHesapla(&insanIrki->birimler[i], insanlar->piyadeler, &toplamSaldiri->piyade_saldirigucu, &toplamSavunma->piyade_savunmagucu,sayac,arastirmaListesi,insanlar,orklar);
        } else if (strcmp(insanIrki->birimler[i].isim, "okcular") == 0) {
            birimSaldiriSavunmaHesapla(&insanIrki->birimler[i], insanlar->okcular, &toplamSaldiri->okcu_saldirigucu, &toplamSavunma->okcu_savunmagucu,sayac,arastirmaListesi, insanlar,orklar);
        } else if (strcmp(insanIrki->birimler[i].isim, "suvariler") == 0) {
            birimSaldiriSavunmaHesapla(&insanIrki->birimler[i], insanlar->suvariler, &toplamSaldiri->suvari_saldirigucu, &toplamSavunma->suvari_savunmagucu,sayac,arastirmaListesi, insanlar,orklar);
        } else if (strcmp(insanIrki->birimler[i].isim, "kusatma_makineleri") == 0) {
            birimSaldiriSavunmaHesapla(&insanIrki->birimler[i], insanlar->kusatma_makineleri, &toplamSaldiri->kusatmaMakineleri_saldirigucu, &toplamSavunma->kusatmaMakineleri_savunmagucu,sayac,arastirmaListesi,insanlar,orklar);
        }
    }
    for (int i = 0; i < orkLegi->birim_sayisi; i++) {
        if (strcmp(orkLegi->birimler[i].isim, "ork_dovusculeri") == 0) {
            birimSaldiriSavunmaHesapla(&orkLegi->birimler[i], orklar->ork_dovusculeri, &toplamSaldiri->ork_dovusculeri_saldirigucu, &toplamSavunma->ork_dovusculeri_savunmagucu,sayac,arastirmaListesi,insanlar, orklar);
        } else if (strcmp(orkLegi->birimler[i].isim, "mizrakcilar") == 0) {
            birimSaldiriSavunmaHesapla(&orkLegi->birimler[i], orklar->mizrakcilar, &toplamSaldiri->mizrakcilar_saldirigucu, &toplamSavunma->mizrakcilar_savunmagucu,sayac,arastirmaListesi,insanlar, orklar);
        } else if (strcmp(orkLegi->birimler[i].isim, "varg_binicileri") == 0) {
            birimSaldiriSavunmaHesapla(&orkLegi->birimler[i], orklar->varg_binicileri, &toplamSaldiri->varg_binicileri_saldirigucu, &toplamSavunma->varg_binicileri_savunmagucu,sayac,arastirmaListesi,insanlar, orklar);
        } else if (strcmp(orkLegi->birimler[i].isim, "troller") == 0) {
            birimSaldiriSavunmaHesapla(&orkLegi->birimler[i], orklar->troller, &toplamSaldiri->troller_saldirigucu, &toplamSavunma->troller_savunmagucu,sayac,arastirmaListesi,insanlar,orklar);
        }
    }
}
void net_hasar_hesapla(saldirigucu toplamSaldiri, savunmaGucu toplamSavunma, NetHasar *hasar) {

    hasar->toplam_insan_saldiri_gucu = toplamSaldiri.piyade_saldirigucu + toplamSaldiri.okcu_saldirigucu +
                                  toplamSaldiri.suvari_saldirigucu + toplamSaldiri.kusatmaMakineleri_saldirigucu;

    hasar->toplam_ork_saldiri_gucu = toplamSaldiri.ork_dovusculeri_saldirigucu + toplamSaldiri.mizrakcilar_saldirigucu +
                               toplamSaldiri.varg_binicileri_saldirigucu + toplamSaldiri.troller_saldirigucu;

    hasar->toplam_insan_savunma_gucu = toplamSavunma.piyade_savunmagucu + toplamSavunma.okcu_savunmagucu +
                                  toplamSavunma.suvari_savunmagucu + toplamSavunma.kusatmaMakineleri_savunmagucu;

    hasar->toplam_ork_savunma_gucu = toplamSavunma.ork_dovusculeri_savunmagucu + toplamSavunma.mizrakcilar_savunmagucu +
                               toplamSavunma.varg_binicileri_savunmagucu + toplamSavunma.troller_savunmagucu;

    if (hasar->toplam_insan_saldiri_gucu > 0) {
            float oran= (1 - (hasar->toplam_ork_savunma_gucu / hasar->toplam_insan_saldiri_gucu));

        hasar->toplam_insan_net_hasar = hasar->toplam_insan_saldiri_gucu * oran;
        if(hasar->toplam_insan_net_hasar<0)hasar->toplam_insan_net_hasar= 0;

    } else {
        hasar->toplam_insan_net_hasar = 0;
    }
    if (hasar->toplam_ork_saldiri_gucu > 0) {

            float oran= (1 - (hasar->toplam_insan_savunma_gucu / hasar->toplam_ork_saldiri_gucu));
         hasar->toplam_ork_net_hasar = hasar->toplam_ork_saldiri_gucu * oran;

          if(hasar->toplam_ork_net_hasar<0)hasar->toplam_ork_net_hasar= 0;

    } else {
        hasar->toplam_ork_net_hasar = 0;
    }
}
void insan_saglik_kaybi_hesapla(Birim *insan_birimler, float ork_net_hasar, InsanBirlikleri *insanlar, NetHasar *hasar, saldirigucu *toplamSaldiri, savunmaGucu *toplamSavunma,FILE *dosya) {
    float toplam_insan_birim_sayisi = insanlar->piyadeler + insanlar->okcular + insanlar->suvariler + insanlar->kusatma_makineleri;
    if (toplam_insan_birim_sayisi > 0) {
        for (int i = 0; i < 4; i++) {
            Birim *birim = &insan_birimler[i];
 if (strcmp(birim->isim, "piyadeler") == 0 && insanlar->piyadeler > 0) {
    if (birim->saglik > 0) {
        float oran = (toplamSavunma->piyade_savunmagucu / hasar->toplam_insan_savunma_gucu);
        float birim_hasar = ork_net_hasar * oran;
        birim->saglik -= birim_hasar / insanlar->piyadeler;
        if (birim->saglik <= 0 || insanlar->piyadeler == 0) {
            printf("%s birimi tamamen yok oldu!\n", birim->isim);
            fprintf(dosya,"%s birimi tamamen yok oldu!\n", birim->isim);
            birim->saglik = 0;
            insanlar->piyadeler=0;
        } else {
            printf("%.0f Tane %s oldu ---- \n", (birim_hasar / birim->max_saglik), birim->isim);
            printf("%s biriminin yeni sagligi: %.2f\n", birim->isim, birim->saglik);
            fprintf(dosya,"%.0f Tane %s oldu ---- \n", (birim_hasar / birim->max_saglik), birim->isim);
            fprintf(dosya,"%s biriminin yeni sagligi: %.2f\n", birim->isim, birim->saglik);
        }
    }
}
else if (strcmp(birim->isim, "okcular") == 0 && insanlar->okcular > 0) {
    if (birim->saglik > 0) {
        float oran = (toplamSavunma->okcu_savunmagucu / hasar->toplam_insan_savunma_gucu);
        float birim_hasar = ork_net_hasar * oran;
            birim->saglik -= birim_hasar / insanlar->okcular;
        if (birim->saglik <= 0) {
            printf("%s birimi tamamen yok oldu!\n", birim->isim);
            fprintf(dosya,"%s birimi tamamen yok oldu!\n", birim->isim);
            birim->saglik = 0;
            insanlar->okcular=0;
        } else {
            printf("%.0f Tane %s oldu ---- \n", (birim_hasar / birim->max_saglik), birim->isim);
            printf("%s biriminin yeni sagligi: %.2f\n", birim->isim, birim->saglik);
            fprintf(dosya,"%.0f Tane %s oldu ---- \n", (birim_hasar / birim->max_saglik), birim->isim);
            fprintf(dosya,"%s biriminin yeni sagligi: %.2f\n", birim->isim, birim->saglik);
        }
    }
}
else if (strcmp(birim->isim, "suvariler") == 0 && insanlar->suvariler > 0) {
    if (birim->saglik > 0) {
        float oran = (toplamSavunma->suvari_savunmagucu / hasar->toplam_insan_savunma_gucu);
        float birim_hasar = ork_net_hasar * oran;
            birim->saglik -= birim_hasar / insanlar->suvariler;
        if (birim->saglik <= 0) {
            printf("%s birimi tamamen yok oldu!\n", birim->isim);
            fprintf(dosya,"%s birimi tamamen yok oldu!\n", birim->isim);
            birim->saglik = 0;
            insanlar->suvariler=0;
        } else {
            printf("%.0f Tane %s oldu ---- \n", (birim_hasar / birim->max_saglik), birim->isim);
            printf("%s biriminin yeni sagligi: %.2f\n", birim->isim, birim->saglik);
            fprintf(dosya,"%.0f Tane %s oldu ---- \n", (birim_hasar / birim->max_saglik), birim->isim);
            fprintf(dosya,"%s biriminin yeni sagligi: %.2f\n", birim->isim, birim->saglik);
        }
    }
}
else if (strcmp(birim->isim, "kusatma_makineleri") == 0 && insanlar->kusatma_makineleri > 0) {
    if (birim->saglik > 0) {
        float oran = (toplamSavunma->kusatmaMakineleri_savunmagucu / hasar->toplam_insan_savunma_gucu);
        float birim_hasar = ork_net_hasar * oran;
            birim->saglik -= birim_hasar / insanlar->kusatma_makineleri;
        if (birim->saglik <= 0) {
            printf("%s birimi tamamen yok oldu!\n", birim->isim);
            fprintf(dosya,"%s birimi tamamen yok oldu!\n", birim->isim);
            birim->saglik = 0;
            insanlar->kusatma_makineleri=0;
        } else {
            printf("%.0f Tane %s oldu ---- \n", (birim_hasar / birim->max_saglik), birim->isim);
            printf("%s biriminin yeni sagligi: %.2f\n", birim->isim, birim->saglik);
            fprintf(dosya,"%.0f Tane %s oldu ---- \n", (birim_hasar / birim->max_saglik), birim->isim);
            fprintf(dosya,"%s biriminin yeni sagligi: %.2f\n", birim->isim, birim->saglik);
        }
    }
}

        }
    }
}
void ork_saglik_kaybi_hesapla(Birim *ork_birimler, float insan_net_hasar, OrkBirlikleri *orklar, NetHasar *hasar, saldirigucu *toplamSaldiri, savunmaGucu *toplamSavunma,FILE *dosya) {
    float toplam_ork_birim_sayisi = orklar->ork_dovusculeri + orklar->mizrakcilar + orklar->varg_binicileri + orklar->troller;
    if (toplam_ork_birim_sayisi > 0) {
        for (int i = 0; i < 4; i++) {
            Birim *birim = &ork_birimler[i];
       if (strcmp(birim->isim, "ork_dovusculeri") == 0 && orklar->ork_dovusculeri > 0) {
    if (birim->saglik > 0) {
        float oran = (toplamSavunma->ork_dovusculeri_savunmagucu / hasar->toplam_ork_savunma_gucu);
        float birim_hasar = insan_net_hasar * oran;
            birim->saglik -= birim_hasar / orklar->ork_dovusculeri;
        if (birim->saglik <= 0) {
            printf("%s birimi tamamen yok oldu!\n", birim->isim);
            fprintf(dosya,"%s birimi tamamen yok oldu!\n", birim->isim);
            birim->saglik = 0;
            orklar->ork_dovusculeri =0;
        } else {
            printf("%.0f Tane %s oldu ---- \n", (birim_hasar / birim->max_saglik), birim->isim);
            printf("%s biriminin yeni sagligi: %.2f\n", birim->isim, birim->saglik);
            fprintf(dosya,"%.0f Tane %s oldu ---- \n", (birim_hasar / birim->max_saglik), birim->isim);
            fprintf(dosya,"%s biriminin yeni sagligi: %.2f\n", birim->isim, birim->saglik);
        }
    }
}
else if (strcmp(birim->isim, "mizrakcilar") == 0 && orklar->mizrakcilar > 0) {
    if (birim->saglik > 0) {
        float oran = (toplamSavunma->mizrakcilar_savunmagucu / hasar->toplam_ork_savunma_gucu);
        float birim_hasar = insan_net_hasar * oran;
            birim->saglik -= birim_hasar / orklar->mizrakcilar;
        if (birim->saglik <= 0 ) {
            printf("%s birimi tamamen yok oldu!\n", birim->isim);
            fprintf(dosya,"%s birimi tamamen yok oldu!\n", birim->isim);
            birim->saglik = 0;
               orklar->mizrakcilar =0;
        } else {
            printf("%.0f Tane %s oldu ---- \n", (birim_hasar / birim->max_saglik), birim->isim);
            printf("%s biriminin yeni sagligi: %.2f\n", birim->isim, birim->saglik);
            fprintf(dosya,"%.0f Tane %s oldu ---- \n", (birim_hasar / birim->max_saglik), birim->isim);
            fprintf(dosya,"%s biriminin yeni sagligi: %.2f\n", birim->isim, birim->saglik);
        }
    }
}
else if (strcmp(birim->isim, "varg_binicileri") == 0 && orklar->varg_binicileri > 0) {
    if (birim->saglik > 0) {
        float oran = (toplamSavunma->varg_binicileri_savunmagucu / hasar->toplam_ork_savunma_gucu);
        float birim_hasar = insan_net_hasar * oran;
            birim->saglik -= birim_hasar / orklar->varg_binicileri;
        if (birim->saglik <= 0) {
            printf("%s birimi tamamen yok oldu!\n", birim->isim);
            fprintf(dosya,"%s birimi tamamen yok oldu!\n", birim->isim);
            birim->saglik = 0;
            orklar->varg_binicileri =0;
        } else {
            printf("%.0f Tane %s oldu ---- \n", (birim_hasar / birim->max_saglik), birim->isim);
            printf("%s biriminin yeni sagligi: %.2f\n", birim->isim, birim->saglik);
            fprintf(dosya,"%.0f Tane %s oldu ---- \n", (birim_hasar / birim->max_saglik), birim->isim);
            fprintf(dosya,"%s biriminin yeni sagligi: %.2f\n", birim->isim, birim->saglik);
        }
    }
}
else if (strcmp(birim->isim, "troller") == 0 && orklar->troller > 0) {
    if (birim->saglik > 0) {
        float oran = (toplamSavunma->troller_savunmagucu / hasar->toplam_ork_savunma_gucu);
        float birim_hasar = insan_net_hasar * oran;
            birim->saglik -= birim_hasar / orklar->troller;
        if (birim->saglik <= 0) {
            printf("%s birimi tamamen yok oldu!\n", birim->isim);
            fprintf(dosya,"%s birimi tamamen yok oldu!\n", birim->isim);
            birim->saglik = 0;
             orklar->troller =0;
        } else {
            printf("%.0f Tane %s oldu ---- \n", (birim_hasar / birim->max_saglik), birim->isim);
            printf("%s biriminin yeni sagligi: %.2f\n", birim->isim, birim->saglik);
            fprintf(dosya,"%.0f Tane %s oldu ---- \n", (birim_hasar / birim->max_saglik), birim->isim);
            fprintf(dosya,"%s biriminin yeni sagligi: %.2f\n", birim->isim, birim->saglik);
        }
    }
}

        }
    }
}
void verileriYazdirSavasinsan(const InsanBirlikleri *insanlar, const OrkBirlikleri *orklar,FILE *dosya) {
    printf("Insan Imparatorlugu Birlikleri:\n");
    fprintf(dosya, "İnsan İmparatorluğu Birlikleri:\n");

    printf("Piyadeler: %d\n", insanlar->piyadeler);
    fprintf(dosya, "Piyadeler: %d\n", insanlar->piyadeler);

    printf("Okcular: %d\n", insanlar->okcular);
    fprintf(dosya, "Okçular: %d\n", insanlar->okcular);

    printf("Suvariler: %d\n", insanlar->suvariler);
    fprintf(dosya, "Süvariler: %d\n", insanlar->suvariler);

    printf("Kusatma Makineleri: %d\n", insanlar->kusatma_makineleri);
    fprintf(dosya, "Kuşatma Makineleri: %d\n\n", insanlar->kusatma_makineleri);

    printf("\nOrk Lejyonu Birlikleri:\n");
    fprintf(dosya, "Ork Lejyonu Birlikleri:\n");

    printf("Ork Dovusculeri: %d\n", orklar->ork_dovusculeri);
    fprintf(dosya, " Ork Dovusculeri: %d\n", orklar->ork_dovusculeri);

    printf("Mızrakcilar: %d\n", orklar->mizrakcilar);
    fprintf(dosya, "Mızrakcilar: %d\n", orklar->mizrakcilar);

    printf("Varg Binicileri: %d\n", orklar->varg_binicileri);
    fprintf(dosya, "Varg Binicileri: %d\n", orklar->varg_binicileri);

    printf("Troller: %d\n", orklar->troller);
    fprintf(dosya, "Troller: %d\n", orklar->troller);
}
void savas_baslat(InsanBirlikleri *insanlar, OrkBirlikleri *orklar, InsanIrki *insanIrki, OrkLegi *orkLegi, saldirigucu *toplamSaldiri, savunmaGucu *toplamSavunma, float toplamInsanBirimSayisi, float toplamOrkBirimSayisi,ArastirmaListesi arastirmaListesi) {
    NetHasar hasar;
    int tur_sayaci = 1;
    FILE *dosya = fopen("sonuc.txt", "w");
    while (toplamInsanBirimSayisi > 0 && toplamOrkBirimSayisi > 0) {
        printf("\nTur: %d\n", tur_sayaci);
        fprintf(dosya,"\nTur: %d\n",tur_sayaci);
        if(tur_sayaci%5==0){
                printf("YORGUNLUK ETKISI--TUM BIRIMLERIN SALDIRI VE SAVUNMA GUCU YUZDE 10 DUSUYOR...\n");
                fprintf(dosya,"YORGUNLUK ETKISI--TUM BIRIMLERIN SALDIRI VE SAVUNMA GUCU YUZDE 10 DUSUYOR...\n");
                gucDusur(insanIrki->birimler, insanIrki->birim_sayisi);
                gucDusur(orkLegi->birimler, orkLegi->birim_sayisi);
}
        if (tur_sayaci % 2 != 0) {
            printf("Insanlar saldiriyor...\n");
            fprintf(dosya,"Insanlar saldiriyor...\n");
            SaldiriSavunmaHesapla(insanIrki, toplamSaldiri, insanlar, toplamSavunma, orklar, orkLegi,tur_sayaci,&arastirmaListesi);
            net_hasar_hesapla(*toplamSaldiri, *toplamSavunma, &hasar);
            ork_saglik_kaybi_hesapla(orkLegi->birimler, hasar.toplam_insan_net_hasar, orklar, &hasar, toplamSaldiri, toplamSavunma,dosya);
        } else {
            printf("Orklar saldiriyor...\n");
            fprintf(dosya,"Orklar saldiriyor...\n");
            SaldiriSavunmaHesapla(insanIrki, toplamSaldiri, insanlar, toplamSavunma, orklar, orkLegi,tur_sayaci,&arastirmaListesi);
            net_hasar_hesapla(*toplamSaldiri, *toplamSavunma, &hasar);
            insan_saglik_kaybi_hesapla(insanIrki->birimler, hasar.toplam_ork_net_hasar, insanlar, &hasar, toplamSaldiri, toplamSavunma,dosya);
        }
        toplamInsanBirimSayisi = insanlar->piyadeler + insanlar->okcular + insanlar->suvariler + insanlar->kusatma_makineleri;
        toplamOrkBirimSayisi = orklar->ork_dovusculeri + orklar->mizrakcilar + orklar->varg_binicileri + orklar->troller;
        verileriYazdirSavasinsan(insanlar,orklar,dosya);
        tur_sayaci++;
        sleep(2);
    }
    if (toplamInsanBirimSayisi > 0) {
        printf("Insanlar kazandi!\n");
        fprintf(dosya,"Insanlar kazandi!\n");
    } else if (toplamOrkBirimSayisi > 0) {
        printf("Orklar kazandi!\n");
        fprintf(dosya,"Orklar kazandi!\n");
    } else {
        printf("savas berabere bitti!\n");
    }
    fclose(dosya);
}
void gucDusur(Birim *birimler, int birim_sayisi) {
    for (int i = 0; i < birim_sayisi; i++) {

        birimler[i].saldiri *= 0.9;
        birimler[i].savunma *= 0.9;
    }
}


void canBariCiz(SDL_Renderer* renderer, int x, int y, float saglik, float max_saglik) {
    int barHeight = 5;
    SDL_Rect arkaPlan = {x, y, GRID_SIZE, barHeight};
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); 
    SDL_RenderFillRect(renderer, &arkaPlan);
    if (max_saglik > 0) { 
        float saglikOrani = saglik/ max_saglik; 
        saglikOrani = saglikOrani < 0 ? 0 : (saglikOrani > 1 ? 1 : saglikOrani); 
        int r = (int)(255 * (1 - saglikOrani)); 
        int g = (int)(255 * saglikOrani); 
        SDL_SetRenderDrawColor(renderer, r, g, 0, 255); 
        SDL_Rect dolgu = {x, y, (int)(GRID_SIZE * saglikOrani), barHeight};
        SDL_RenderFillRect(renderer, &dolgu);
    }
}
void askerleriYerleştir(SDL_Renderer* renderer, SDL_Texture* textures[], InsanBirlikleri insanBirlikleri, InsanIrki insanIrki, int startX, int startY, TTF_Font* font) {
    int offsetY = 0;
    int offsetX = 10;
    int x = startX;
    int maxRows = 20;
    int colOffset = GRID_SIZE * 3;
    int currentColumn = 0;
    if (insanBirlikleri.piyadeler > 0) {
        int birimSayisi = insanBirlikleri.piyadeler;
        while (birimSayisi > 0) {
            int yerleştirilen = (birimSayisi > 100) ? 100 : birimSayisi;
            SDL_Rect destRect = { x + currentColumn * colOffset + offsetX, startY + offsetY, GRID_SIZE, GRID_SIZE };
            SDL_RenderCopy(renderer, textures[0], NULL, &destRect);
            char text[10];
            snprintf(text, sizeof(text), "%d", yerleştirilen);
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, (SDL_Color){0, 0, 0, 255});
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture) {
                    SDL_Rect textRect = { x + currentColumn * colOffset + offsetX - 50, startY + offsetY + (GRID_SIZE - 30) / 2, 50, 30 };
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
            float saglik = insanIrki.birimler[0].saglik;
            float max_saglik = insanIrki.birimler[0].max_saglik;
            canBariCiz(renderer, x + currentColumn * colOffset + offsetX, startY + offsetY + GRID_SIZE - 10, 
                       saglik > 0 ? saglik : 0, max_saglik);
            birimSayisi -= yerleştirilen;
            offsetY += GRID_SIZE;
            if (offsetY >= maxRows * GRID_SIZE) {
                offsetY = 0;
                currentColumn++;
            }
        }
    }
    if (insanBirlikleri.okcular > 0) {
        int birimSayisi = insanBirlikleri.okcular;
        while (birimSayisi > 0) {
            int yerleştirilen = (birimSayisi > 100) ? 100 : birimSayisi;
            SDL_Rect destRect = { x + currentColumn * colOffset + offsetX, startY + offsetY, GRID_SIZE, GRID_SIZE };
            SDL_RenderCopy(renderer, textures[1], NULL, &destRect);
            char text[10];
            snprintf(text, sizeof(text), "%d", yerleştirilen);
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, (SDL_Color){0, 0, 0, 255});
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture) {
                    SDL_Rect textRect = { x + currentColumn * colOffset + offsetX - 50, startY + offsetY + (GRID_SIZE - 30) / 2, 50, 30 };
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
            canBariCiz(renderer, x + currentColumn * colOffset + offsetX, startY + offsetY + GRID_SIZE - 10, 
                       insanIrki.birimler[1].saglik > 0 ? insanIrki.birimler[1].saglik : 0,
                       insanIrki.birimler[1].max_saglik);
            birimSayisi -= yerleştirilen;
            offsetY += GRID_SIZE;
            if (offsetY >= maxRows * GRID_SIZE) {
                offsetY = 0;
                currentColumn++;
            }
        }
    }
    if (insanBirlikleri.suvariler > 0) {
        int birimSayisi = insanBirlikleri.suvariler;
        while (birimSayisi > 0) {
            int yerleştirilen = (birimSayisi > 100) ? 100 : birimSayisi;
            SDL_Rect destRect = { x + currentColumn * colOffset + offsetX, startY + offsetY, GRID_SIZE, GRID_SIZE };
            SDL_RenderCopy(renderer, textures[2], NULL, &destRect);
            char text[10];
            snprintf(text, sizeof(text), "%d", yerleştirilen);
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, (SDL_Color){0, 0, 0, 255});
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture) {
                    SDL_Rect textRect = { x + currentColumn * colOffset + offsetX - 50, startY + offsetY + (GRID_SIZE - 30) / 2, 50, 30 };
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
            canBariCiz(renderer, x + currentColumn * colOffset + offsetX, startY + offsetY + GRID_SIZE - 10, 
                       insanIrki.birimler[2].saglik > 0 ? insanIrki.birimler[2].saglik : 0,
                       insanIrki.birimler[2].max_saglik);

            birimSayisi -= yerleştirilen;
            offsetY += GRID_SIZE;

            if (offsetY >= maxRows * GRID_SIZE) {
                offsetY = 0;
                currentColumn++;
            }
        }
    }
    if (insanBirlikleri.kusatma_makineleri > 0) {
        int birimSayisi = insanBirlikleri.kusatma_makineleri;
        while (birimSayisi > 0) {
            int yerleştirilen = (birimSayisi > 100) ? 100 : birimSayisi;
            SDL_Rect destRect = { x + currentColumn * colOffset + offsetX, startY + offsetY, GRID_SIZE, GRID_SIZE };
            SDL_RenderCopy(renderer, textures[3], NULL, &destRect);
            char text[10];
            snprintf(text, sizeof(text), "%d", yerleştirilen);
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, (SDL_Color){0, 0, 0, 255});
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture) {
                    SDL_Rect textRect = { x + currentColumn * colOffset + offsetX - 50, startY + offsetY + (GRID_SIZE - 30) / 2, 50, 30 };
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
            canBariCiz(renderer, x + currentColumn * colOffset + offsetX, startY + offsetY + GRID_SIZE - 10, 
                       insanIrki.birimler[3].saglik,
                       insanIrki.birimler[3].max_saglik);
            birimSayisi -= yerleştirilen;
            offsetY += GRID_SIZE;
            if (offsetY >= maxRows * GRID_SIZE) {
                offsetY = 0;
                currentColumn++;
            }
        }
    }
}
void orklariYerleştir(SDL_Renderer* renderer, SDL_Texture* textures[], OrkBirlikleri orklar, OrkLegi orkLegi, int startX, int startY, TTF_Font* font) {
    int offsetY = 0;
    int x = startX;
    int maxRows = 20;
    int colOffset = GRID_SIZE * 3;
    int currentColumn = 0;
    if (orklar.varg_binicileri > 0 && orkLegi.birim_sayisi > 0) {
        int birimSayisi = orklar.varg_binicileri;
        while (birimSayisi > 0) {
            int yerleştirilen = (birimSayisi > 100) ? 100 : birimSayisi;
            SDL_Rect destRect = { x + currentColumn * colOffset, startY + offsetY, GRID_SIZE, GRID_SIZE };
            SDL_RenderCopy(renderer, textures[2], NULL, &destRect);
            char text[10];
            snprintf(text, sizeof(text), "%d", yerleştirilen);
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, (SDL_Color){0, 0, 0, 255});
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture) {
                    SDL_Rect textRect = { x + currentColumn * colOffset - 50, startY + offsetY + (GRID_SIZE - 30) / 2, 50, 30 };
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
            float saglik = orkLegi.birimler[0].saglik;  // Varg binicilerinin sağlık bilgisi
            float max_saglik = orkLegi.birimler[0].max_saglik;
            canBariCiz(renderer, x + currentColumn * colOffset, startY + offsetY + GRID_SIZE - 10, 
                       saglik > 0 ? saglik : 0, max_saglik);
            birimSayisi -= yerleştirilen;
            offsetY += GRID_SIZE;
            if (offsetY >= maxRows * GRID_SIZE) {
                offsetY = 0;
                currentColumn++;
            }
        }
    }
    if (orklar.troller > 0 && orkLegi.birim_sayisi > 1) {
        int birimSayisi = orklar.troller;
        while (birimSayisi > 0) {
            int yerleştirilen = (birimSayisi > 100) ? 100 : birimSayisi;
            SDL_Rect destRect = { x + currentColumn * colOffset, startY + offsetY, GRID_SIZE, GRID_SIZE };
            SDL_RenderCopy(renderer, textures[3], NULL, &destRect);
            char text[10];
            snprintf(text, sizeof(text), "%d", yerleştirilen);
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, (SDL_Color){0, 0, 0, 255});
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture) {
                    SDL_Rect textRect = { x + currentColumn * colOffset - 50, startY + offsetY + (GRID_SIZE - 30) / 2, 50, 30 };
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
            float saglik = orkLegi.birimler[1].saglik;  // Trollerin sağlık bilgisi
            float max_saglik = orkLegi.birimler[1].max_saglik;
            canBariCiz(renderer, x + currentColumn * colOffset, startY + offsetY + GRID_SIZE - 10, 
                       saglik > 0 ? saglik : 0, max_saglik);
            birimSayisi -= yerleştirilen;
            offsetY += GRID_SIZE;
            if (offsetY >= maxRows * GRID_SIZE) {
                offsetY = 0;
                currentColumn++;
            }
        }
    }
    if (orklar.ork_dovusculeri > 0 && orkLegi.birim_sayisi > 2) {
        int birimSayisi = orklar.ork_dovusculeri;
        while (birimSayisi > 0) {
            int yerleştirilen = (birimSayisi > 100) ? 100 : birimSayisi;
            SDL_Rect destRect = { x + currentColumn * colOffset, startY + offsetY, GRID_SIZE, GRID_SIZE };
            SDL_RenderCopy(renderer, textures[0], NULL, &destRect);
            char text[10];
            snprintf(text, sizeof(text), "%d", yerleştirilen);
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, (SDL_Color){0, 0, 0, 255});
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture) {
                    SDL_Rect textRect = { x + currentColumn * colOffset - 50, startY + offsetY + (GRID_SIZE - 30) / 2, 50, 30 };
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
            float saglik = orkLegi.birimler[2].saglik;  // Ork dövüşçülerinin sağlık bilgisi
            float max_saglik = orkLegi.birimler[2].max_saglik;
            canBariCiz(renderer, x + currentColumn * colOffset, startY + offsetY + GRID_SIZE - 10, 
                       saglik > 0 ? saglik : 0, max_saglik);

            birimSayisi -= yerleştirilen;
            offsetY += GRID_SIZE;

            if (offsetY >= maxRows * GRID_SIZE) {
                offsetY = 0;
                currentColumn++;
            }
        }
    }
    if (orklar.mizrakcilar > 0 && orkLegi.birim_sayisi > 3) {
        int birimSayisi = orklar.mizrakcilar;
        while (birimSayisi > 0) {
            int yerleştirilen = (birimSayisi > 100) ? 100 : birimSayisi;
            SDL_Rect destRect = { x + currentColumn * colOffset, startY + offsetY, GRID_SIZE, GRID_SIZE };
            SDL_RenderCopy(renderer, textures[1], NULL, &destRect);
            char text[10];
            snprintf(text, sizeof(text), "%d", yerleştirilen);
            SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, (SDL_Color){0, 0, 0, 255});
            if (textSurface) {
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                if (textTexture) {
                    SDL_Rect textRect = { x + currentColumn * colOffset - 50, startY + offsetY + (GRID_SIZE - 30) / 2, 50, 30 };
                    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                    SDL_DestroyTexture(textTexture);
                }
                SDL_FreeSurface(textSurface);
            }
            float saglik = orkLegi.birimler[3].saglik;  // Mızrakçıların sağlık bilgisi
            float max_saglik = orkLegi.birimler[3].max_saglik;
            canBariCiz(renderer, x + currentColumn * colOffset, startY + offsetY + GRID_SIZE - 10, 
                       saglik > 0 ? saglik : 0, max_saglik);

            birimSayisi -= yerleştirilen;
            offsetY += GRID_SIZE;

            if (offsetY >= maxRows * GRID_SIZE) {
                offsetY = 0;
                currentColumn++;
            }
        }
    }
}
void izgarayiCiz(SDL_Renderer* renderer, int satirSayisi, int sutunSayisi) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
    for (int i = 0; i <= satirSayisi; i++) {
        SDL_RenderDrawLine(renderer, 0, i * GRID_SIZE, sutunSayisi * GRID_SIZE, i * GRID_SIZE);
    }
    for (int j = 0; j <= sutunSayisi; j++) {
        SDL_RenderDrawLine(renderer, j * GRID_SIZE, 0, j * GRID_SIZE, satirSayisi * GRID_SIZE);
    }
}
void ekranaPngYukle(SDL_Renderer* renderer, const char* dosyaYolu, int x, int y) {
    SDL_Surface* surface = IMG_Load(dosyaYolu);
    if (!surface) {
        printf("PNG yüklenemedi: %s\n", IMG_GetError());
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dstRect;
    dstRect.x = x; 
    dstRect.y = y; 
    dstRect.w = surface->w; 
    dstRect.h = surface->h; 
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}
void ekranaYaziYaz(SDL_Renderer* renderer, const char* yazi, int x, int y, SDL_Color renk) {
    TTF_Font* font = TTF_OpenFont("arial.ttf", 24); // Font dosyasını ve boyutunu belirt
    SDL_Surface* surface = TTF_RenderText_Solid(font, yazi, renk);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("Doku oluşturulamadı: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        TTF_CloseFont(font);
        return;
    }
    SDL_Rect dstRect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    TTF_CloseFont(font);
}
void savas_sirasinda(SDL_Renderer* renderer) {
    (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048));
    Mix_Music *muzik = Mix_LoadMUS("savas.mp3"); 
    Mix_PlayMusic(muzik, -1); // Sonsuz döngüde çal (-1 ile)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);  
    izgarayiCiz(renderer, 20, 20); 
    SDL_Color siyah = { 0, 0, 0, 255 };
    ekranaYaziYaz(renderer, "   VS  ", 360, 100, siyah);
    ekranaYaziYaz(renderer, "SAVAS DEVAM EDIYOR SEVGILI OYUNCUMUZ", 140, 250, siyah);
    ekranaYaziYaz(renderer, "BAKALIM KAZANAN TARAF HANGI TARAF OLUCAK", 140, 450, siyah);
    ekranaPngYukle(renderer, "Metehan.png", 10, 10); 
    ekranaPngYukle(renderer, "Samur.png", 550, 4); 
    SDL_RenderPresent(renderer); 
}
void savas_sonrasi(SDL_Renderer* renderer, SDL_Texture* humanTextures[], SDL_Texture* orkTextures[], 
                   InsanBirlikleri insanlar, OrkBirlikleri orklar, InsanIrki insanIrki, OrkLegi orkLegi, 
                   TTF_Font* font) {
    Mix_CloseAudio(); 
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); 
    SDL_RenderClear(renderer); 
    izgarayiCiz(renderer, 20, 20); 
    askerleriYerleştir(renderer, humanTextures, insanlar, insanIrki, 110, 0, font);
    orklariYerleştir(renderer, orkTextures, orklar, orkLegi, 640, 0, font);
    SDL_RenderPresent(renderer);  
}
int main(int argc, char* args[]) {
    TTF_Init();
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    
    int font_size = 26; 
    TTF_Font* font = TTF_OpenFont("Arial.ttf", font_size);
    SDL_Window* window = SDL_CreateWindow("Orklar vs İnsanlar", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); 
    SDL_RenderClear(renderer);  
    izgarayiCiz(renderer, 20, 20); 
    SDL_Surface* iconSurface = IMG_Load("Makrog_Savas_Beyi.png"); 
    SDL_SetWindowIcon(window, iconSurface); 
    SDL_FreeSurface(iconSurface); 
    SDL_Texture* humanTexturesLoaded[4];
    SDL_Texture* orkTexturesLoaded[4];

    for (int i = 0; i < 4; i++) {
        humanTexturesLoaded[i] = loadTexture(humanTextures[i], renderer);
        if (humanTexturesLoaded[i] == NULL) {
            printf("Error loading texture: %s\n", humanTextures[i]);
        }
    }
    for (int i = 0; i < 4; i++) {
        orkTexturesLoaded[i] = loadTexture(orkTextures[i], renderer);
        if (orkTexturesLoaded[i] == NULL) {
            printf("Error loading texture: %s\n", orkTextures[i]);
        }
    }
    char* jsonVeri = dosyaOku("unit_types.json");
    InsanIrki insanIrki;
    OrkLegi orkLegi;
    InsanBirlikleri insanlar = {0};
    OrkBirlikleri orklar = {0};
    if (jsonVeri != NULL) {
        jsonAyristirVeAta(jsonVeri, &insanIrki, &orkLegi);
        free(jsonVeri);
    }
    char* kahramanVeri = dosyaOku("heroes.json");
    if (kahramanVeri == NULL) {
        return 1; 
    }
    InsanKahramanlari insanKahramanlari = { .kahraman_sayisi = 0 };
    OrkKahramanlari orkKahramanlari = { .kahraman_sayisi = 0 };
    json_verilerini_isle(kahramanVeri, &insanKahramanlari, &orkKahramanlari);
    free(kahramanVeri);
    char* arastirmaVeri = dosyaOku("research.json");
    ArastirmaListesi arastirmaListesi = {0};
    if (arastirmaVeri != NULL) {
        const char* arastirma_adlari[] = {"savunma_ustaligi", "saldiri_gelistirmesi", "elit_egitim", "kusatma_ustaligi"};
        arastirmalariAyrisir(arastirmaVeri, arastirma_adlari, 4, arastirmaListesi.arastirmalar, &arastirmaListesi.arastirma_sayisi);
        free(arastirmaVeri);
    }
    Irk insan_irki;
    Irk ork_lejyonu;
    json_verilerini_isle_canavar("creatures.json", &insan_irki, &ork_lejyonu);

    jsonOku("https://yapbenzet.org.tr/10.json", &insanlar, &orklar);
    verileriYazdir(&insanlar, &orklar);

    printf("\n\n");
    saldirigucu toplamSaldiri;
    savunmaGucu toplamSavunma;
    NetHasar hasar;
    float toplamInsanBirimSayisi = insanlar.piyadeler + insanlar.okcular + insanlar.suvariler + insanlar.kusatma_makineleri;
    float toplamOrkBirimSayisi = orklar.ork_dovusculeri + orklar.mizrakcilar + orklar.varg_binicileri + orklar.troller;

    int running = 1;
    int savasBaslatildi = 0; 
    int savasBitti = 0; 
    printf("SAVAŞI BASLATMAK ICIN ENTER TUSUNA BASINIZ:");
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            if (event.type == SDL_KEYDOWN) {
                // Enter tuşuna basıldığında savaşı başlat
                if (event.key.keysym.sym == SDLK_RETURN && !savasBaslatildi) {
                    kahramanlarin_etkisini_uygula(insanlar, orklar, insanIrki.birimler, insanIrki.birim_sayisi, orkLegi.birimler, orkLegi.birim_sayisi, &insanKahramanlari, &orkKahramanlari);
                    canavarlarin_etkisini_uygula(insanlar, orklar, insanIrki.birimler, insanIrki.birim_sayisi, orkLegi.birimler, orkLegi.birim_sayisi, &insan_irki, &ork_lejyonu);
                    arastirma_niteliklerini_uygula(insanlar, orklar, insanIrki.birimler, insanIrki.birim_sayisi, orkLegi.birimler, orkLegi.birim_sayisi, &arastirmaListesi);
                    savas_sirasinda(renderer);
                    savas_baslat(&insanlar, &orklar, &insanIrki, &orkLegi, &toplamSaldiri, &toplamSavunma, toplamInsanBirimSayisi, toplamOrkBirimSayisi, arastirmaListesi);       
                    savasBaslatildi = 1;
                    savasBitti = 1; 
                }
            }
        }
        if (!savasBaslatildi) {
            SDL_RenderClear(renderer);
            izgarayiCiz(renderer, 20, 20);
            askerleriYerleştir(renderer, humanTexturesLoaded, insanlar, insanIrki, 110, 0, font);
            orklariYerleştir(renderer, orkTexturesLoaded, orklar, orkLegi, 640, 0, font);
            SDL_RenderPresent(renderer);
        }
        if (savasBitti) {
            SDL_RenderClear(renderer);
            savas_sonrasi(renderer, humanTexturesLoaded, orkTexturesLoaded, insanlar, orklar, insanIrki, orkLegi, font);
            savasBitti = 0; 
        }
    }
    for (int i = 0; i < 4; i++) {
        SDL_DestroyTexture(humanTexturesLoaded[i]);
        SDL_DestroyTexture(orkTexturesLoaded[i]);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit(); 
    return 0;
}