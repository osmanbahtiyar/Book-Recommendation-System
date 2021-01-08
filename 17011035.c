#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//benzerliklerin tutulacagi similarity matrisinin her bir elemani bu tipte olacak
//hangi kullanicinin benzerligi ne kadar oldugu tutuluyor
typedef struct
{
    int user_id;
    float similarity;
} Sim_node;

//csv dosyasinin okunarak train icin kullanilacak kullanicilarin train_data, test için kullanilacak kullanicilarin test_data icerisine atilmesi, kitap isimlerinin book_names icerisine islemleri bu fonksiyonda yapılıyor
void readFromCSV(int **train_data, int train_size, int **test_data, int test_size, char **book_names, int book_count);

//iki kullanicinin birbiri ile olan benzerligini hesaplayan fonksiyon
float calculateSimilarity(int *user1, int *user2, int book_count);

//verilen dizinin verilen indexinden sonrasini bir saga kaydiran fonksiyon
void shiftRight(Sim_node *arr, int n, int index);

//verilen kullaniciya en cok benzeyen k adet kullanicinin tutuldugu k boyutlu diziye insert islemi yapan fonksiyon
void insertToSimilarityArray(Sim_node *similarity_arr, int k, float new_sim, int new_id);

//verilen kullaniciya en benzer k adet kullaniciyi bulan fonksiyon
void findKSimilarUsers(int *new_user, Sim_node *nu_similarity, int **train_data, int train_size, int book_count, int k);

//verilen kullanicinin okudugu kitaplara verdigi ortalama puani hesaplayan fonksiyon
float calculateAverageRating(int *user, int book_count);

//verilen kullanici icin verilen kitaba kac puan verecegini tahmin eden fonksiyon
float makePrediction(int *nu, int book_no, Sim_node *nu_similarity, int **train_data, int book_count, int k);

int main()
{
    int train_size;
    int test_size;
    int book_count;
    int k;

    int **train_data;
    int **test_data;
    char **book_names;
    Sim_node **similarity;

    printf("Train data kullanici sayisini giriniz: ");
    scanf("%d", &train_size);
    printf("Test data kullanici sayisini giriniz: ");
    scanf("%d", &test_size);
    printf("Kitap sayisini giriniz: ");
    scanf("%d", &book_count);
    printf("En benzer kac kisiyi istiyorsunuz: ");
    scanf("%d", &k);

    train_data = (int **)calloc(train_size, sizeof(int *));
    test_data = (int **)calloc(test_size, sizeof(int *));
    book_names = (char **)calloc(book_count, sizeof(char *));
    similarity = (Sim_node **)calloc(test_size, sizeof(Sim_node *));
    if (train_data == NULL || test_data == NULL || book_names == NULL || similarity == NULL)
    {
        fprintf(stderr, "Allocation Error\n");
        exit(1);
    }

    int i;
    for (i = 0; i < train_size; i++)
    {
        train_data[i] = (int *)calloc(book_count, sizeof(int));
        if (train_data[i] == NULL)
        {
            fprintf(stderr, "Allocation Error\n");
            exit(1);
        }
    }
    for (i = 0; i < test_size; i++)
    {
        test_data[i] = (int *)calloc(test_size, sizeof(int));
        if (test_data[i] == NULL)
        {
            fprintf(stderr, "Allocation Error\n");
            exit(1);
        }
    }
    for (i = 0; i < book_count; i++)
    {
        book_names[i] = (char *)calloc(50, sizeof(char));
        if (book_names[i] == NULL)
        {
            fprintf(stderr, "Allocation Error\n");
            exit(1);
        }
    }
    for (i = 0; i < test_size; i++)
    {
        similarity[i] = (Sim_node *)calloc(k, sizeof(Sim_node));
        if (similarity[i] == NULL)
        {
            fprintf(stderr, "Allocation Error\n");
            exit(1);
        }
    }

    readFromCSV(train_data, train_size, test_data, test_size, book_names, book_count);
    printf("\n\n");
    for (i = 0; i < test_size; i++)
    {
        findKSimilarUsers(test_data[i], similarity[i], train_data, train_size, book_count, k);
    }
    for (i = 0; i < test_size; i++)
    {
        int j;
        printf("NU%d icin benzerlik oranlari:\n", i + 1);
        for (j = 0; j < k; j++)
        {
            printf("%d. en yakin user: U%d, benzerlik: %f\n", j + 1, similarity[i][j].user_id + 1, similarity[i][j].similarity);
        }
    }

    printf("\n\n");
    for (i = 0; i < test_size; i++)
    {
        int j;
        float max_rate = -1;
        int max_book;
        printf("NU%d icin muhtemel kitap puanlari:\n", i + 1);
        for (j = 0; j < book_count; j++)
        {
            if (test_data[i][j] == 0)
            {
                float rate = makePrediction(test_data[i], j, similarity[i], train_data, book_count, k);
                printf("%s kitabi icin beklenen puan : %f\n", book_names[j], rate);
                if (rate > max_rate)
                {
                    max_rate = rate;
                    max_book = j;
                }
            }
        }
        printf("\033[0;31m"
               "Biz %s kitabini oneriyoruz!\n"
               "\033[0m",
               book_names[max_book]);
        printf("\n");
    }

    return 0;
}

//verilen kullanicinin okudugu kitaplara verdigi ortalama puani return eden fonksiyon
// @user -> puanı hesaplanacak kullanici (verilen puanlari tuttugun matrisin bu kullanicinin verdigi puanlara ait olan satiri)
// @book_count -> veri setim icerisinde kac farkli kitap bulundugu
float calculateAverageRating(int *user, int book_count)
{
    //dongu degiskeni
    int i;
    //kullanicinin okudugu kitaplarin sayisi
    int user_count = 0;
    //kullanicinin okudugu kitaplara verdigi puanlarin ortalamasini tutan degisken
    float avg = 0;
    //veri seti icerisindeki her kitap icin
    for (i = 0; i < book_count; i++)
    {
        //eger kullanici kitabi okudu ise
        if (user[i] != 0)
        {
            //kitaba verdigi puani ortalamayi tutan degiskene ekleyen fonksiyon
            avg += user[i];
            //kullanicinin okudugu kitap sayisini arttiran fonksiyon
            user_count++;
        }
    }
    //toplam puanin okunan kitap sayisina bolumunu return eder
    return avg / user_count;
}

//verilen kullanicinin verilen kitaba kac puan verecegini hesaplayan fonksiyon
//@nu -> kitaba verecegi puanin tahmin edilecegi kullanicinin kitaplara verdigi puanlar arayi
//@book_no -> kullanicinin verdigi puanin tahmin edilecegi kitap
//@nu_similarity -> tum benzerliklerin tutuldugu matrisin ilgili kullaniciya ait satiri
//@train_data -> kullanicilarin kitaplara verdigi puanlari tutan matris
//@book_count -> dataset icerisindeki kitap sayisi
//@k -> en yakin kac kullanicinin tutulacagi
float makePrediction(int *nu, int book_no, Sim_node *nu_similarity, int **train_data, int book_count, int k)
{
    //dongu degiskeni
    int i;
    //kitaba verilecek puan
    float rate = 0;
    //tahmin edilecek kullanicinin ortalama verdigi puan
    float avg_nu = calculateAverageRating(nu, book_count);
    //kullanilan formulun pay kismi
    float term1 = 0;
    //kullanilan formulun payda kismi
    float term2 = 0;
    //en benzer k kisi icin dongu
    for (i = 0; i < k; i++)
    {
        // en benzer i. user'in ortalama puaninin hesaplanmasi
        float avg_u = calculateAverageRating(train_data[nu_similarity[i].user_id], book_count);
        //en benzer i. user ile benzerligi hesaplayan fonksiyon
        float sim = calculateSimilarity(nu, train_data[nu_similarity[i].user_id], book_count);
        //formulun ust kismi
        term1 += sim * (train_data[nu_similarity[i].user_id][book_no] - avg_u);
        //formulun alt kismi
        term2 += sim;
    }
    //pay / paydayi return eden fonksiyon
    return avg_nu + (term1 / term2);
}

//en yakin k kisiyi bulan fonksiyon
//@new_user -> kendisine en yakin kisilerin hesaplanacagi kisi
//@nu_similarity -> hesaplanan benzerliklerin yerlestirilecegi matrisin bu kullaniciya ait satiri
//@train_data -> tum kullanicilar ve kitaplara verdigi puanlar matrisi
//@train_size -> verdigi puanlar uzerinden hesap yapilacak kullanici sayisi
//@book_count -> dataset icerisindeki kitap sayisi
//@k -> en yakin kac adet kullanicinin bulunacagi
void findKSimilarUsers(int *new_user, Sim_node *nu_similarity, int **train_data, int train_size, int book_count, int k)
{
    //dongu degiskeni
    int i;
    //k sutunlu similarity matrisinin tum sutunlari icin dongu
    for (i = 0; i < k; i++)
    {
        //basta tum benzerlikleri -1 olarak ayarla
        nu_similarity[i].similarity = -1;
    }

    //train data icerisindeki her kullanici icin dongu
    for (i = 0; i < train_size; i++)
    {
        //train datanin i. kullanicisi icin similarity hesaplanmasi
        float sim = calculateSimilarity(new_user, train_data[i], book_count);
        //hesaplanan similarity'i kullanicinin similarity dizisine insert eden fonksiyon
        insertToSimilarityArray(nu_similarity, k, sim, i);
    }
}

//verilen degeri k elemanli diziye insert eden fonksiyon
void insertToSimilarityArray(Sim_node *similarity_arr, int k, float new_sim, int new_id)
{
    //dongu degiskeni
    int i;
    //k elemanli dizinin
    for (i = 0; i < k; i++)
    {
        //yeni gonderilen similarity dizinin bu elemanindan buyuk mu diye bak
        if (new_sim > similarity_arr[i].similarity)
        {
            //eger daha buyuk ise bu indexten itibaren her elemani saga kaydir
            shiftRight(similarity_arr, k, i);
            //yeni gelen similarity'i buraya ekle
            similarity_arr[i].similarity = new_sim;
            //yeni gelen similarity'nin hangi kullanici ile oldugu bilgisini tut
            similarity_arr[i].user_id = new_id;
            return;
        }
    }
}

//verilen diziyi verilen indexten itibaren bir eleman saga kaydiran fonksiyon
//@arr -> similarity dizisinin bir satiri
//@n -> dizinin eleman sayisi
//@index -> hangi indexten itibaren kaydiracagim bilgisi
void shiftRight(Sim_node *arr, int n, int index)
{
    //dongu degiskeni
    int i;
    //dizinin sonundan verilen indexe kadar dongu
    for (i = n - 1; i > index; i--)
    {
        //degerleri bir sagdaki goze yaz
        arr[i].similarity = arr[i - 1].similarity;
        arr[i].user_id = arr[i - 1].user_id;
    }
}

float calculateSimilarity(int *user1, int *user2, int book_count)
{
    int i;
    float sim = 0;
    float avg_user1 = 0;
    int user1_count = 0;
    float avg_user2 = 0;
    int user2_count = 0;
    for (i = 0; i < book_count; i++)
    {
        if (user1[i] != 0)
        {
            avg_user1 += user1[i];
            user1_count++;
        }
        if (user2[i] != 0)
        {
            avg_user2 += user2[i];
            user2_count++;
        }
    }
    avg_user1 /= user1_count;
    avg_user2 /= user2_count;

    float cov = 0;
    float std_user1 = 0;
    float std_user2 = 0;
    for (i = 0; i < book_count; i++)
    {
        if (user1[i] != 0 && user2[i] != 0)
        {
            cov += (user1[i] - avg_user1) * (user2[i] - avg_user2);
            std_user1 += (user1[i] - avg_user1) * (user1[i] - avg_user1);
            std_user2 += (user2[i] - avg_user2) * (user2[i] - avg_user2);
        }
    }
    std_user1 = sqrt(std_user1);
    std_user2 = sqrt(std_user2);
    sim = cov / (std_user1 * std_user2);
    return sim;
}

//csv dosyasinin okunarak train icin kullanilacak kullanicilarin train_data, test için kullanilacak kullanicilarin test_data icerisine atilmesi, kitap isimlerinin book_names icerisine islemleri bu fonksiyonda yapılıyor
// @train_data -> karsilastirma isleminde kullanilacak kullanicilerin tutulduğu matris
// @train_size -> karsilastirma isleminde kullanilacak kullanicilarin sayisi
// @test_data -> sonucların test edilecegi kullanicilarin bilgilerini tutan matris
// @test_size -> sonuclarin test edilecegi kullanici sayisi
// @book_names -> kitap isimlerinin tutulacagi string array
// @book_count -> kitap sayisi
void readFromCSV(int **train_data, int train_size, int **test_data, int test_size, char **book_names, int book_count)
{
    FILE *fp = fopen("RecomendationDataSet.csv", "r");
    if (fp == NULL)
    {
        fprintf(stderr, "File error");
        exit(1);
    }

    char line[500];

    fgets(line, sizeof(line), fp);
    char *token;
    int i = 0;
    token = strtok(line, "\n");
    token = strtok(line, ",");
    token = strtok(NULL, ",");
    while (token != NULL)
    {
        strcpy(book_names[i++], token);
        token = strtok(NULL, ",");
    }

    for (i = 0; i < train_size; i++)
    {
        fgets(line, sizeof(line), fp);
        token = strtok(line, "\n");
        token = strtok(line, ",");
        token = strtok(NULL, ",");
        int j = 0;
        while (token != NULL)
        {
            train_data[i][j++] = atoi(token);
            token = strtok(NULL, ",");
        }
    }

    fgets(line, sizeof(line), fp);

    for (i = 0; i < test_size; i++)
    {
        fgets(line, sizeof(line), fp);
        token = strtok(line, "\n");
        token = strtok(line, ",");
        token = strtok(NULL, ",");
        int j = 0;
        while (token != NULL)
        {
            test_data[i][j++] = atoi(token);
            token = strtok(NULL, ",");
        }
    }
}
