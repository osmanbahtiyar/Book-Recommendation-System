#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MAX_LENGTH 50

//her kullanicinin en benzer kisileri bir linked list yapisinda tutulacaktir
typedef struct sim_node
{
    //kullanicinin bizim bu kullanici ile similaritysi
    float similarity;
    //bu similarity'nin ait oldugu kullanicinin id'si
    int id;
    //linked list next
    struct sim_node *next;
} SimNode;

//her kullanici bir struct olarak tutulacaktir
typedef struct user
{
    //okudugu kitaplara verdigi puanlarin oldugu bir integer array
    //kitaps sayisina gore dinamik acilacak
    int *books;
    //benzer kullanicilarin tutuldugu k uzunluklu linked list'in head'i
    SimNode *similarities;
} User;

//U ile baslayan kullanicilarin tutuldugu bir array
User **train_data = NULL;
//NU ile baslayan kullanicilarin tutuldugu array
User **test_data = NULL;
//kitaplarin isimlerinin tutuldugu string array
char **book_names = NULL;
//kitapların sayisi
int book_count = 0;
//kac tane U ile baslayan kullanici oldugunun sayisi
int train_count = 0;
//kac tane NU ile baslayan kullanici oldugunun sayisi
int test_count = 0;
//en benzer kac kullanicinin gosterilecegini gosteren k sayisi
int k;

//verilen dosyadaki kitap ve kullanici bilgilerini okuyan fonksiyon
void readFromCSV();
//yeni kitap eklendikce kitap isimlerinin tutuldugu string arrayini reallocate eden fonksiyon
void expandBookNames();
//User tipinde bir struct allocate eden fonksiyon
User *createNewUser();
//yeni U kullanicisi eklendikce U kullanicilarinin tutuldugu array'i reallocate eden fonksiyon
void expandTrainData();
//yeni NU kullanicisi eklendikce NU kullanicilarinin tutuldugu array'i reallocate eden fonksiyon
void expandTestData();
//verilen kullanicinin kitaplara verdigi ortalama puanlari bulan fonksiyon
float calculateAverageRating(User *user);
//verilen iki kullanicinin verilen formule gore benzerliklerini hesaplayan fonksiyon
float calculateSimilarity(User *user1, User *user2);
//verilen kullaniciya en benzer k kisiyi bulan fonksiyon
void findKSimilarUsers(User *user);
//verilen kullanicinin similarity listesine verilen yeni sim ve id degerlerini sırali insert eden fonksiyon
void insertToSimilarityList(User *user, float new_sim, int new_id);
//verilen kullanicinin verilen kitaba kac puan verecegini tahmin eden fonksiyon
float makePrediction(User *nu, int book_no);

int main()
{
    //verilen dosyayi oku ve uygun kullanicilari olustur
    readFromCSV();

    //kullanici gecerli bir deger girmedigi surece soran bir do-while dongusu
    do
    {
        //kullanicidan en yakin kac kisiyi hesaplamak istedigi bilgisinin alinmasi
        printf("Please enter the similar user count (k) bigger than 0: ");
        scanf("%d", &k);
        //eger kullanici 0 dan kucuk bir sayi girdi ise uyar
        if (k <= 0)
        {
            printf("k must be positive integer number!\n");
        }
        //eger kullanici k degerini bizim U ile baslayan train user sayimizdan fazla girdi ise uyar
        if (k > train_count)
        {
            printf("Not that many users!!\n");
        }
        //kosullar saglanmadigi surece sor
    } while (k <= 0 || k > train_count);

    //kullanicinin hangi NU icin degerleri istedigini tutacak degisken
    int id;
    do
    {
        //kullanicidan hangi NU icin degerleri istedigini sor
        printf("Please enter the test data user id\nFor example please enter only 2 for NU2: ");
        scanf("%d", &id);
        //eger girdigi deger dosyada yoksa uyar
        if (id > test_count)
        {
            printf("There is no user like NU%d\n", id);
            //id'nin gecersiz oldugunu belirtmek icin -1 yap
            id = -1;
        }
        //id positive integer olmak zorunda degilse uyar
        else if (id <= 0)
        {
            printf("ID has to be positive integer\n");
            //id'nin gecersiz oldugunu belirtmek icin -1 yap
            id = -1;
        }
        //id gecersiz oldugu surece don
    } while (id == -1);
    //kullanicidan alinan id 1 den basliyor ancak bizim indexlerimiz 0'dan basliyor o yuzden 1 azalt
    id--;

    //i dongu degiskeni
    int i;
    //Test_data arrayimdeki her user icin
    for (i = 0; i < test_count; i++)
    {
        //her test user'in en benzer k kisini bul
        findKSimilarUsers(test_data[i]);
    }
    //her test kullanicisi icin
    for (i = 0; i < test_count; i++)
    {
        //j dongu degiskeni
        int j;
        //kacinci kullanicinin similarity skorunu gosterdigimi kullaniciya bildir
        printf("Similarity scores for NU%d:\n", i + 1);
        //kullaniciya en benzer k kisinin skorlarini gostermek icin linked list uzerinde ilerleyecek index pointer
        SimNode *temp = test_data[i]->similarities;
        //k kere don
        for (j = 0; j < k; j++)
        {
            //kullanici ile benzer olan kisinin id'si ve kullanici ile benzerlik oranlari
            printf("\tU%d, similarity->%f\n", temp->id + 1, temp->similarity);
            //similarities linked listinin bir sonraki elemanina gec
            temp = temp->next;
        }
        printf("\n");
    }

    //tum test kullanicilari icin dongu
    for (i = 0; i < test_count; i++)
    {
        //j dongu degiskeni
        int j;
        //en yuksek rate bizim onerecegimiz kitap bunu baslangicta dusuk bir deger olan -1 den baslattim
        float max_rate = -1;
        //onerilecek kitabin kitaplar arrayindeki indexini tutan degisken
        int max_book;
        //hangi kullanici icin tahmin edilen oranlari gosterdigimi kullaniciya bildir
        printf("Predicted rates for NU%d\n", i + 1);
        //her kitap icin don
        for (j = 0; j < book_count; j++)
        {
            //eger kullanici bu kitabi okumamis ise
            if (test_data[i]->books[j] == 0)
            {
                //tahmin edilen puani hesapla
                float rate = makePrediction(test_data[i], j);
                //kullaniciya bu kitabin adini ve tahmini puanini soyle
                printf("\tPredicted rate for %s -> %f\n", book_names[j], rate);
                //eger bu oran benim max oranindan buyuk ise
                if (rate > max_rate)
                {
                    //yeni max oran olarak bunu belirle
                    max_rate = rate;
                    //yeni en yuksek oranli kitabin indexi olarak bu kitabin indexini belirle
                    max_book = j;
                }
            }
        }
        //en yuksek puanli kitabi kullaniciya biz bu kitabi oneriyoruz diye bildir
        printf("We suggest book %s with predicted rate %f\n", book_names[max_book], max_rate);
        printf("\n");
    }

    //kullanicinin ozellikle sectigi kullaniciyi kullaniciya hatirlat
    printf("You selected NU%d\n", id + 1);
    //max puan olarak dusuk bir deger belirle
    float max_rate = -1;
    //max puanli kitabin indexi olacak
    int max_book;
    //tum kitaplar icin dongu
    for (i = 0; i < book_count; i++)
    {
        //eger kullanici bu kitabi okumamis ise
        if (test_data[id]->books[i] == 0)
        {
            //tahmini puani hesapla
            float rate = makePrediction(test_data[id], i);
            //bu kitap icin tahmini puani kullaniciya bildir
            printf("\tPredicted rate for %s -> %f\n", book_names[i], rate);
            //eger bu kitabin puani max orandan daha buyuk ise
            if (rate > max_rate)
            {
                //bu orani yeni max oran olarak belirle
                max_rate = rate;
                //bu kitabin indexini yeni max index olarak belirle
                max_book = i;
            }
        }
    }
    //en yuksek olasi puanli kitabi kullaniciya bildir
    printf("We suggest book %s with predicted rate %f\n", book_names[max_book], max_rate);

    return 0;
}

//alinan kullanicinin alinan numarali kitaba tahmini olarak kac puan verecegini hesaplayan fonksiyon
//@nu -> kitaba verecegi puan hesaplanacak kullanici
//@book_no -> kullanicinin verecegi puan hesaplanacak kitabin numarasi
float makePrediction(User *nu, int book_no)
{
    //i dongu degiskeni
    int i;
    //tahmin edilecek kullanicinin kitaplara verdigi ortalama puani tutan degisken
    float avg_nu = calculateAverageRating(nu);
    //formulun pay kismi
    float term1 = 0;
    //formulun payda kismi
    float term2 = 0;
    //kullanicinin similarity linked listinde dolasmak icin index pointer
    SimNode *train_user = nu->similarities;
    //kullanicinin similarities linked listinin her elemani icin dongu
    for (i = 0; i < k; i++)
    {
        //kullanicinin similaritie linked listindeki kullanicinin id'sine sahip kullanicinin ortalmasinin hesaplanmasi
        float avg_u = calculateAverageRating(train_data[train_user->id]);
        //bu kullanici ile similarity listesindeki kullanicinin benzerliklerinin hesaplanmasi
        float sim = calculateSimilarity(nu, train_data[train_user->id]);
        //formulun pay kismi, similarity listesindeki kullanicinin bu kitaba verdigi puan ile ortalama puaninin farkinin bizim kullanicimiz ile olan benzerligi ile carpilmasi
        term1 += sim * (train_data[train_user->id]->books[book_no] - avg_u);
        //paydaya iki kullanicinin benzerliginin eklenmesi
        term2 += sim;
        //similarity listindeki bir sonraki kullaniciya gecilmesi
        train_user = train_user->next;
    }
    //bizim kullanicimizin ortalama puani ile formulun pay ve paydasinin bolumunun toplanmasi
    return avg_nu + (term1 / term2);
}

//verilen kullanicinin similarity linked listine new_id numarali kisinin similarity bilgilerinin insert edilmesi
//@user-> similarity linked listine insert yapilacak kullanici
//@new_id -> verilen kullanicinin similarity listine id'si ve similarity'si insert edilecek kullanicinin id'si
//@new_sim -> verilen kullanicinin similarity listine id'si ve similarity'si insert edilecek kullanicinin similarity'si
void insertToSimilarityList(User *user, float new_sim, int new_id)
{
    //yeni bir similarity node'u allocate et
    SimNode *newNode = (SimNode *)malloc(sizeof(SimNode));
    //allocation kontrolu
    if (newNode == NULL)
    {
        printf("Allocation Error\n");
        exit(1);
    }
    //yeni olusturdugum node'a similarity degerini yaz
    newNode->similarity = new_sim;
    //yeni olusturdugum koda id degerini yaz
    newNode->id = new_id;

    //eger kullanicinin similarity listi bos ise veya ilk elemaninin similarity degeri bizim insert edecegimiz degerden kucuk ise yeni node'u listenin basina insert et
    if (user->similarities == NULL || user->similarities->similarity <= newNode->similarity)
    {
        //yeni node'un next'ini head yap
        newNode->next = user->similarities;
        //yeni head'i yeni node yap
        user->similarities = newNode;
        //fonksiyonu bitir
        return;
    }

    //similarity listinde kac adim ilerdegimizi sayan degisken
    int count = 0;
    //similarity listinde index pointer olarak kullancagimiz degisken
    SimNode *current = user->similarities;
    //bundan sonra node yoksa ve bundan sonraki node'un similarity'si insert edilecek node'dan dusuk ise ve listede k adimdan az ilerlendi ise
    while (current->next != NULL && current->next->similarity > newNode->similarity && count < k)
    {
        //adim sayisini bir arttir
        count++;
        //bir sonraki node'a gec
        current = current->next;
    }
    //eger adim sayisi k dan kucuk ise
    if (count < k)
    {
        //yeni node'u buraya ekle
        //yeni node'un nextine current'in nextini ata
        newNode->next = current->next;
        //current->next'ine yeni node'u ata
        current->next = newNode;
    }
    //eger k adimdan fazla ilerlendi ise en yuksek k degerden daha dusuk demektir eklemeye gerek yok
}

//verilen kullaniciya en benzer k adet kullaniciyi bulan fonksiyon
//@user -> kendisine benzer kullanicilarin bulunacagi kullanici
void findKSimilarUsers(User *user)
{
    //i dongu degiskeni
    int i;
    //train datadaki her U kullanicisi icin dongu
    for (i = 0; i < train_count; i++)
    {
        //verilen kullanicinin i. train kullanicisi ile olan benzerligini hesapla
        float sim = calculateSimilarity(user, train_data[i]);
        //hesaplanan benzerligi verilen kullanicinin similarity listine insert eden fonksiyonu cagir
        insertToSimilarityList(user, sim, i);
    }
}

//verilen iki kullanicinin birbirine olan benzerligini hesaplayan fonksiyon
//@user1 -> benzerlik hesaplanacak 1. kullanici
//@user2 -> benzerlik hesaplanacak 2. kullanici
float calculateSimilarity(User *user1, User *user2)
{
    //i dongu degiskeni
    int i;
    //similarity degerini tutacak degisken
    float sim = 0;
    //1.kullanicinin ortalama puanini tutan degisken
    float avg_user1 = calculateAverageRating(user1);
    //2.kullanicinin ortalama puanini tutan degisken
    float avg_user2 = calculateAverageRating(user2);
    //iki kullanicinin covariance degerini tutan degisken
    float cov = 0;
    //1. kullanicinin standart sapmasini tutan degisken
    float std_user1 = 0;
    //2.kullanicinin standart sapmasini tutan degisken
    float std_user2 = 0;
    //her bir kitap icin dongu
    for (i = 0; i < book_count; i++)
    {
        //eher her iki kullanici da bu kitabi okudu ise
        if (user1->books[i] != 0 && user2->books[i] != 0)
        {
            //1.kullanicinin bu kitaba verdigi puandan ortalama puanini cikar, 2.kullanicinin bu kitaba verdigi puandan ortalama puanini cikar ve bu iki degeri carpip co degerine ekle
            cov += (user1->books[i] - avg_user1) * (user2->books[i] - avg_user2);
            //1.kullanicinin bu kitaba verdigi puandan ortalama puani cikar ve karesini alip standart sapmasina ekle
            std_user1 += (user1->books[i] - avg_user1) * (user1->books[i] - avg_user1);
            //2.kullanicinin bu kitaba verdigi puandan ortalama puani cikar ve karesini alip standart sapmasina ekle
            std_user2 += (user2->books[i] - avg_user2) * (user2->books[i] - avg_user2);
        }
    }
    //standart sapma icin karekokunu al
    std_user1 = sqrt(std_user1);
    //standart sapma icin karekokunu al
    std_user2 = sqrt(std_user2);
    //cov degerini standart sapmalarin carpimina bol
    sim = cov / (std_user1 * std_user2);
    //bu degeri return et
    return sim;
}

//verilen kullanicinin kitaplara verdigi ortalama puani hesaplayan fonksiyon
//@user -> ortalamsi hesaplanacak kullanici
float calculateAverageRating(User *user)
{
    //i dongu degiskeni
    int i;
    //kullanicinin okudugu kitap sayisini tutacak degisken
    int user_book_count = 0;
    //kullanicinin verdigi ortalama puani tutacak degisken
    float user_avg = 0;

    //her kitap icin dongu
    for (i = 0; i < book_count; i++)
    {
        //eger kullanici bu kitabi okudu ise
        if (user->books[i] != 0)
        {
            //kullanicinin puanina bu kitaba verdigi puani ekle
            user_avg += user->books[i];
            //kullanicinin okudugu kitap sayisini 1 arttir
            user_book_count++;
        }
    }
    //kullanicinin verdigi toplam puani okudugu kitap sayisina bol ve return et
    return user_avg / user_book_count;
}

//RecomendationDataSet.csv dosyasini uygun sekilde okuyan fonksiyon
void readFromCSV()
{
    //dosyayi ac
    FILE *fp = fopen("RecomendationDataSet.csv", "r");
    //dosya acildi mi kontrol et
    if (fp == NULL)
    {
        fprintf(stderr, "File error");
        exit(1);
    }

    //dosyadan satir satir alinacak veri icin bir buffer
    char line[1000];

    //dosyanin ilk satirini al
    fgets(line, sizeof(line), fp);
    //satiri token etmek icin kullanilacak pointer
    char *token;
    //satiri \n e gore token et
    token = strtok(line, "\n");
    //satiri , e gore token et
    token = strtok(line, ","); //dosyanin basindaki bosluk var
    //bir daha , e gore token et
    token = strtok(NULL, ","); //icinde ilk kitabin adi var
    //tum satir token edilene kadar dongu
    while (token != NULL)
    {
        //kitap sayisini 1 arttir
        book_count++;
        //kitap isimlerinin tutulacagi string arrayinin boyutunu 1 arttir
        expandBookNames();
        //alinan kitap adini kitap isimlerinin tutuldugu string arrayine kopyala
        strcpy(book_names[book_count - 1], token);
        //bir sonraki elemani token et
        token = strtok(NULL, ",");
    }

    //kacinci train kullanicisinin bilgilerini token ettigimi tutan degisken
    int i_train = 0;
    //kacinci test kullancisinin bilgilerini token ettigimi tutan degisken
    int i_test = 0;
    //j dongu degiskeni
    int j;
    //dosyaninin sonuna gelene kadar dongu
    while (fgets(line, sizeof(line), fp))
    {
        //satirin sonundaki \n i at
        token = strtok(line, "\n");
        //satirin ilk elemanini token et bu kullanici adi
        token = strtok(line, ",");
        //kullanici adi U ile basliyor ise train dataya NU ile basliyorsa test dataya atilacak ilk harfini user_type olarak al
        char user_type = token[0];
        //kullanicinin ilk kitaba verdigi puan
        token = strtok(NULL, ",");
        //eger kullanicinin turu U veya u ise
        if (user_type == 'U' || user_type == 'u')
        {
            //train data sayacini 1 arttir
            train_count++;
            //train data arrayinin boyutunu bir arttir
            expandTrainData();
            //train data arrayinin yeni acilan gozune yeni bir kullanici allocate edip yerlestir
            train_data[train_count - 1] = createNewUser();
            //j kacinci kitabi okudugumu tutan degisken
            j = 0;
            //satirin sonuna kadar
            while (token != NULL)
            {
                //okunan kitap puanini kullanicinin kitaplar arrayine at puani stringten integer'a cevirmek icin atoi kullanildi
                train_data[i_train]->books[j] = atoi(token);
                //kitap numarasini arttir
                j++;
                //yeni kitabi al
                token = strtok(NULL, ",");
            }
            //train edilen kullanici sayisini arttir
            i_train++;
        }
        //eger kullanici turu NU ise
        else
        {
            //test data sayacini arttir
            test_count++;
            //test data arrayini buyut
            expandTestData();
            //test datanin yeni yerine yeni bir kullanici yarat
            test_data[test_count - 1] = createNewUser();
            //kacinci kitabi okudugumu tutan degisken
            j = 0;
            //satirin sonuna kadar token et
            while (token != NULL)
            {
                //okunan kitap puanini kullanicinin kitaplar arrayine at puani stringten integer'a cevirmek icin atoi kullanildi
                test_data[i_test]->books[j] = atoi(token);
                //kitap numarasini arttir
                j++;
                //yeni kitabi al
                token = strtok(NULL, ",");
            }
            //test edilen kullanici sayisini arttir
            i_test++;
        }
    }
}

//test datanin tutuldugu array'in boyutunu reallocate eden fonksiyon
void expandTestData()
{
    //realloc isleminin guvenligi icin gecici degiskene allocation yapildi
    User **tmp_data = (User **)realloc(test_data, test_count * sizeof(User *));
    //bir sorun var ise hata ver ve programi bitir
    if (tmp_data == NULL)
    {
        printf("Allocation Error\n");
        exit(1);
    }
    //bir sorun yok ise
    else
    {
        //test dataya yeni olusturulan degiskeni at
        test_data = tmp_data;
    }
}

//train datanin tutuldugu array'in boyutunu reallocate eden fonksiyon
void expandTrainData()
{
    //realloc isleminin guvenligi icin gecici degiskene allocation yapildi
    User **tmp_data = (User **)realloc(train_data, train_count * sizeof(User *));
    //bir sorun var ise hata ver ve programi bitir
    if (tmp_data == NULL)
    {
        printf("Allocation Error\n");
        exit(1);
    }
    //bir sorun yok ise
    else
    {
        //train dataya yeni olusturulan degiskeni at
        train_data = tmp_data;
    }
}

//kitap isimlerinin tutuldugu char matrisini reallocate eden fonksiyon
void expandBookNames()
{
    //realloc isleminin guvenligi icin gecici degiskene allocation yapildi
    char **new_book_names = (char **)realloc(book_names, book_count * sizeof(char *));
    //bir sorun var ise hata ver ve programi kapat
    if (new_book_names == NULL)
    {
        printf("Allocation Error\n");
        exit(1);
    }
    //bir sorun yok ise
    else
    {
        //book_names'e yeni olusturulan degiskeni at
        book_names = new_book_names;
    }

    //book_names matrisinin yeni eklenen satirini allocate et
    book_names[book_count - 1] = (char *)malloc(MAX_LENGTH * sizeof(char));
    //bir sorun var mi kontrol et
    if (book_names[book_count - 1] == NULL)
    {
        printf("Allocation Error\n");
        exit(1);
    }
}

//yeni bir User yapisi olusturan fonksiyon
User *createNewUser()
{
    //bir User pointer allocate et
    User *newUser = (User *)malloc(sizeof(User));
    //dogru allocate edildi mi kontrolu
    if (newUser == NULL)
    {
        printf("Allocation Error\n");
        exit(1);
    }

    //yeni kullanicinin kitaplara verdigi puanlarin tutulacagi books arrayini kitap sayisi kadar allocate et
    newUser->books = (int *)malloc(book_count * sizeof(int));
    //dogru allocate edildi mi kontrol et
    if (newUser->books == NULL)
    {
        printf("Allocation Error\n");
        exit(1);
    }
    //yeni kullanicinin similarities linked listine baslangicta null ata
    newUser->similarities = NULL;
    //yeni kullaniciyi return et
    return newUser;
}
