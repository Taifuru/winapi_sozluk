#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int kelimeArama(FILE *, FILE *, unsigned char [20], char [20], char [20], int , int *);
void siralama(char [][20], int);
int dosyaOkuma(FILE *, char [][20]);
void dosyaYazma(FILE *, FILE *, FILE *, char [][20], fpos_t *, int );
void degistir(char [], char []);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//Dosyalardaki imleçlerinin konumlarýný tutmak için deðiþkenler tanýmlandý
//fpos_t konum, konumTr, konumIng;
fpos_t konum, konumTr, konumIng;

//Dosya degiskenlerinin tanimlanmasi
//FILE *sozluk, *turkce, *ingilizce;
FILE *sozluk, *turkce, *ingilizce;

//textbox tan alýnan metin
char alinanyazi[20];
//Dosya okumadan sonra alýnan türkçe kelimeler
char siralanacakKelimeler[100][20];

//Aranan kelimenin karþýlýðý bulunamadýðý zaman çýkan sonuç
char hata_mesaji[] = "Aradýðýnýz Kelime Sözlükte Bulunmamaktadýr!";

//Aranan kelimenin kaçýncý satýrda olduðu
int arananKelimeninYeri = 0;

//aramam sonrasý bulunan kelimeler
char turkceKelime[20] = "";
char ingKelime[20] = "";

//Ekranda oluþturulmuþ pencerelerin deðiþkenleri
HWND buton1, buton2, textbox1, textbox2, yaziyeri[9];

/* The 'main' function of Win32 GUI programs: this is where execution starts */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//Degiskenlere degerleri atilarak islemin yapilip yapilmadigiyla alakali kontroller
	//ve kullaniciya geri bildirim
	if( (sozluk = fopen("sozluk.txt", "r") ) == NULL)
	{
		printf("sozluk.txt Açýlamadý! \n");
		getch();
		exit(0);
	}
	else if( ( turkce = fopen("turkce.txt", "w") ) == NULL)
	{
		printf("turkce.txt Açýlamadý! \n");
		getch();
		exit(0);
	}
	else if ( ( ingilizce = fopen("ingilizce.txt", "w") ) == NULL )
	{
		printf("ingilizce.txt Açýlamadý! \n");
		getch();
		exit(0);
	}
	
	//sozluk.txt deki imleç konumu daha sonra kullanýlmak üzere alýnýyor
	fgetpos(sozluk, &konum);
	
	//Dosya okumasý sonrasý kaç kelime okunduðu tutuluyor
	int uzunluk = dosyaOkuma(sozluk, siralanacakKelimeler);
	
	//Türkçe kelimelerin alfabetik sýraya konulmasý
	siralama(siralanacakKelimeler, uzunluk);
	
	//Alfabetik sýraya dizilen kelimelerin ait olduklarý dosyalara yazýlmasý
	dosyaYazma(sozluk,  turkce,  ingilizce, siralanacakKelimeler,&konum, uzunluk);
	
	//Dosyalar yazma modunda açýlmýþtý. O nedenle kapatýp tekrar okuma modunda açýyoruz
	fclose(turkce);
	fclose(ingilizce);
	fclose(sozluk);
	
	if( ( turkce = fopen("turkce.txt", "r") ) == NULL)
	{
		printf("turkce.txt Açýlamadý! \n");
		exit(0);
	}
	else if ( ( ingilizce = fopen("ingilizce.txt", "r") ) == NULL )
	{
		printf("ingilizce.txt Açýlamadý! \n");
		exit(0);
	}
	
	//Dosyalardaki konumlar daha sonra tekrar kullanýlmak üzere alýnýyor
	fgetpos(ingilizce, &konumIng);
	fgetpos(turkce, &konumTr);
	
	WNDCLASSEX wc; /* A properties struct of our window */
	HWND anaPencere; /* A 'HANDLE', hence the H, or a pointer to our window */
	MSG Msg; /* A temporary location for all messages */

	/* zero out the struct and set the stuff we want to modify */
	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc; /* This is where we will send messages to */
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	
	/* White, COLOR_WINDOW is just a #define for a system color, try Ctrl+Clicking it */
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+25);
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION); /* Load a standard icon */
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION); /* use the name "A" to use the project icon */

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	anaPencere = CreateWindowEx(WS_EX_CLIENTEDGE,"WindowClass","Türkçe-Ýngilizce / Ýngilizce-Türkçe",WS_VISIBLE|WS_MINIMIZEBOX|WS_SYSMENU,
		CW_USEDEFAULT, /* x */
		CW_USEDEFAULT, /* y */
		640, /* width */
		300, /* height */
		NULL,NULL,hInstance,NULL);

	if(anaPencere == NULL) {
		MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	/*
		This is the heart of our program where all input is processed and 
		sent to WndProc. Note that GetMessage blocks code flow until it receives something, so
		this loop will not produce unreasonably high CPU usage
	*/
	while(GetMessage(&Msg, NULL, 0, 0) > 0) { /* If no error is received... */
		TranslateMessage(&Msg); /* Translate key codes to chars if present */
		DispatchMessage(&Msg); /* Send it to WndProc */
	}
	
	fclose(sozluk);
	fclose(turkce);
	fclose(ingilizce);
	
	return Msg.wParam;
}

/* This is where all the input to the window goes to */
LRESULT CALLBACK WndProc(HWND anaPencere, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message) {
		
		case WM_CREATE:
		{
			//gerekli pencerelerin daha program baþlangýcýnda oluþturulmasý
			buton1 = CreateWindow("BUTTON","Türkçe-->Ýngilizce",WS_VISIBLE|WS_CHILD|WS_BORDER,
														20,10,400,25, anaPencere,(HMENU) 1,NULL,NULL);
			
			textbox1 = CreateWindow("EDIT","Aranacak Türkçe kelimeyi yazýnýz", WS_VISIBLE|WS_CHILD|WS_BORDER,
														20,40,400,25,anaPencere,NULL,NULL,NULL);
			
			textbox2 = CreateWindow("EDIT","Aranacak Ýngilizce kelimeyi yazýnýz", WS_VISIBLE|WS_CHILD|WS_BORDER,
														20,100,400,25,anaPencere,NULL,NULL,NULL);											
			
			buton2 = CreateWindow("BUTTON","Ýngilizce-->Türkçe",WS_VISIBLE|WS_CHILD|WS_BORDER,
														20,70,400,25,anaPencere,(HMENU) 2,NULL,NULL);
			
			break;
		}
		
		case WM_COMMAND: {
			
			switch(LOWORD(wParam))
			{
				case 1:
				{
					//her butona basýlýþta ekranda ki sonuçlarýn yenisi yazmak için silinmesi
					int i;
					for(i=0;i<9;i++)
						DestroyWindow(yaziyeri[i]);
					
					//Textbox ta bulunan verinin deðiþkene atanmasý
					int deger=0;
					deger=GetWindowText(textbox1,&alinanyazi[0],20);
					
					int durum = 0;
					//Ýmleç her aramadan önce dosya baþýna alýnýyor
					fsetpos(ingilizce, &konumIng);
					fsetpos(turkce, &konumTr);
				
					//arama sonucuna göre ekrana yazýlacak sonuçlar deðerlendirilicek
					durum = kelimeArama(turkce, ingilizce, turkceKelime, ingKelime, alinanyazi, 1, &arananKelimeninYeri);
					
					if(durum == 1)
					{
						yaziyeri[0] = CreateWindow("STATIC","turkce.txt dosyasýnda aradýðýnýz",WS_VISIBLE|WS_CHILD|WS_BORDER,20,130,255,25,anaPencere,NULL,NULL,NULL);
						yaziyeri[1] = CreateWindow("STATIC", turkceKelime ,WS_VISIBLE|WS_CHILD|WS_BORDER,275,130,90,25,anaPencere,NULL,NULL,NULL);
						yaziyeri[2] = CreateWindow("STATIC", " kelimesi " ,WS_VISIBLE|WS_CHILD|WS_BORDER,365,130,80,25,anaPencere,NULL,NULL,NULL);
						char sayi[3];
						sayi[0] = '0' + arananKelimeninYeri / 100;
	  					sayi[1] = '0' + ( arananKelimeninYeri - ( (arananKelimeninYeri / 100) * 100) ) / 10;
	  					sayi[2] = '0' + arananKelimeninYeri % 10;
						yaziyeri[3] = CreateWindow("STATIC",sayi, WS_VISIBLE|WS_CHILD|WS_BORDER,445,130,30,25,anaPencere,NULL,NULL,NULL);
						yaziyeri[4] = CreateWindow("STATIC",". kelimedir.", WS_VISIBLE|WS_CHILD|WS_BORDER,475,130,90,25,anaPencere,NULL,NULL,NULL);
						
						yaziyeri[5] = CreateWindow("STATIC",turkceKelime,WS_VISIBLE|WS_CHILD|WS_BORDER,20,155,90,25,anaPencere,NULL,NULL,NULL);
						yaziyeri[6] = CreateWindow("STATIC", " kelimesinin Ýngilizce karþýlýðý " ,WS_VISIBLE|WS_CHILD|WS_BORDER,110,155,255,25,anaPencere,NULL,NULL,NULL);
						yaziyeri[7] = CreateWindow("STATIC", ingKelime ,WS_VISIBLE|WS_CHILD|WS_BORDER,365,155,90,25,anaPencere,NULL,NULL,NULL);
						yaziyeri[8] = CreateWindow("STATIC"," kelimesidir.", WS_VISIBLE|WS_CHILD|WS_BORDER,455,155,100,25,anaPencere,NULL,NULL,NULL);
					}
					else
					{
						yaziyeri[0] = CreateWindow("STATIC", hata_mesaji ,WS_VISIBLE|WS_CHILD|WS_BORDER,20,130,360,25,anaPencere,NULL,NULL,NULL);
					}
					break;
			}
				
				case 2:
				{
					int i;
					for(i=0;i<9;i++)
						DestroyWindow(yaziyeri[i]);
						
					int asd=0;
					asd=GetWindowText(textbox2,&alinanyazi[0],20);
					
					int durum = 0;
					fsetpos(ingilizce, &konumIng);
					fsetpos(turkce, &konumTr);
					durum = kelimeArama(turkce, ingilizce, turkceKelime, ingKelime, alinanyazi, 2, &arananKelimeninYeri);
					
					if( durum == 1)
					{
						yaziyeri[0] = CreateWindow("STATIC","ingilizce.txt dosyasýnda aradýðýnýz",WS_VISIBLE|WS_CHILD|WS_BORDER,20,130,270,25,anaPencere,NULL,NULL,NULL);
						yaziyeri[1] = CreateWindow("STATIC", ingKelime ,WS_VISIBLE|WS_CHILD|WS_BORDER,290,130,90,25,anaPencere,NULL,NULL,NULL);
						yaziyeri[2] = CreateWindow("STATIC", " kelimesi " ,WS_VISIBLE|WS_CHILD|WS_BORDER,380,130,80,25,anaPencere,NULL,NULL,NULL);
						char sayi[3];
						sayi[0] = '0' + arananKelimeninYeri / 100;
	  					sayi[1] = '0' + ( arananKelimeninYeri - ( (arananKelimeninYeri / 100) * 100) ) / 10;
	  					sayi[2] = '0' + arananKelimeninYeri % 10;
						yaziyeri[3] = CreateWindow("STATIC",sayi, WS_VISIBLE|WS_CHILD|WS_BORDER,460,130,30,25,anaPencere,NULL,NULL,NULL);
						yaziyeri[4] = CreateWindow("STATIC",". kelimedir.", WS_VISIBLE|WS_CHILD|WS_BORDER,490,130,90,25,anaPencere,NULL,NULL,NULL);
						
						yaziyeri[5] = CreateWindow("STATIC",ingKelime,WS_VISIBLE|WS_CHILD|WS_BORDER,20,155,90,25,anaPencere,NULL,NULL,NULL);
						yaziyeri[6] = CreateWindow("STATIC", " kelimesinin Türkçe karþýlýðý " ,WS_VISIBLE|WS_CHILD|WS_BORDER,110,155,255,25,anaPencere,NULL,NULL,NULL);
						yaziyeri[7] = CreateWindow("STATIC", turkceKelime ,WS_VISIBLE|WS_CHILD|WS_BORDER,365,155,90,25,anaPencere,NULL,NULL,NULL);
						yaziyeri[8] = CreateWindow("STATIC"," kelimesidir.", WS_VISIBLE|WS_CHILD|WS_BORDER,455,155,100,25,anaPencere,NULL,NULL,NULL);
					}
					else
					{
						yaziyeri[0] = CreateWindow("STATIC", hata_mesaji,WS_VISIBLE|WS_CHILD|WS_BORDER,20,130,360,25,anaPencere,NULL,NULL,NULL);
					}
					break;
				}
			}
			
			break;
		}
		/* Upon destruction, tell the main thread to stop */
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		
		/* All other messages (a lot of them) are processed using default procedures */
		default:
			return DefWindowProc(anaPencere, Message, wParam, lParam);
	}
	return 0;
}

int dosyaOkuma(FILE *sozluk1, char siralanacakKelimeler[][20])
{
	//Sozluk.txt icindeki kelimelerin ayrilmasi
	char temp[20];
	int okunanKelimeSayisi = 0;
	int index = 0;
	
	while( !feof( sozluk1 ))//Dosya sonuna gelinip gelinmediðin kontrolü
	{
		//Sadece türkçe kelimleri okumak için 0 dan baþladýðýmýzdan 2 nin katlarýnda okuma yaptýk
		//böylece sadece türkçe kelimeleri okuduk
		if( index % 2 == 0)
		{
			fscanf(sozluk1, "%s", siralanacakKelimeler[okunanKelimeSayisi]);
			okunanKelimeSayisi++;
		}
		//Ýngilizce olanlarý da imlecin ilerlemesi için geçici bir diziye attýk
		else
		{
			fscanf(sozluk1, "%s", temp);
		}
		
		index++;
	}
	
	//Kaç kelime okunduðunu return ediyoruz
	return okunanKelimeSayisi;
}

void degistir(char kelime1[20], char kelime2[20])
{
	char temp[20];
	
	//2 kelimenin yer deðiþtirmesi
	//1 i geçici olana atýyoruz , 2 yi 1 e atýyoruz
	//geçiciyi 2 ye atýyoruz
	strcpy(temp, kelime1);
	strcpy(kelime1, kelime2);
	strcpy(kelime2, temp);
}

//  http://en.wikipedia.org/wiki/Bubble_sort
void siralama(char kelimeler[][20], int uzunluk1)
{
	int index1;
	int index2;
	int bekci = 1;
	
	for(index1 = 0; index1 < uzunluk1; index1++)
	{
		bekci = 0;
		for(index2  = 0; index2 < uzunluk1; index2++)
		{
			if( strcasecmp( kelimeler[index1], kelimeler[index2]) < 0)
			{
				degistir(kelimeler[index1], kelimeler[index2]);
				bekci = 1;
			}
		}
		
		if(bekci == 0)
		{
			break;
		}
	}
}

void indexArama(FILE *sozluk2, char turkce[20], char ingKelime[20])
{
	char temp[20];
	int index = 0;
	
	while( !feof( sozluk2 ) )
	{	
		if( index % 2 == 0 )
		{
			fscanf(sozluk2, "%s", temp);
			
			//sozluk.txt den okunan kelime ile Türkçe kelime karþýlaþtýrýlýyor
			if( strcasecmp(turkce, temp) == 0 )
			{
				//Eðer kelime bulunduysa bir sonraki kelime Ýngilizce olan olacaðý için
				//Bir okuma daha yapýlýp ingilizce kelime bulunmuþ oluyor
				fscanf(sozluk2, "%s", ingKelime);
			}
		}
		else
		{
			//Ýmlecin yer deðiþtirmesi için geçici okuma yapýlýyor
			fscanf(sozluk2, "%s", &temp);
		}
		
	}
}

void dosyaYazma(FILE *sozluk1, FILE *turkce1, FILE *ingilizce1, char kelimeler[][20], fpos_t *konum, int uzunluk)
{
	char ingKelime[20];
	int index = 0;
	
	while(index < uzunluk)
	{
		//Kelimeyi turkce.txt ye yazýyoruz
		fprintf(turkce1, "%s\n", kelimeler[index]);
		
		//sozluk.txt deki imleci en baþa alýyoruz
		fsetpos(sozluk1, konum);
		
		//Türkçe kelimenin çevirisi olan Ýngilizce kelimeyi arýyoruz
		indexArama(sozluk1, kelimeler[index], ingKelime);
		
		//Bulduðumuz Ýngilizce kelimeyi ingilizce.txt ye yazýyoruz
		fprintf(ingilizce1, "%s\n", ingKelime);
		
		index++;
	}
}

int kelimeArama(FILE *turkce3, FILE *ingilizce3, unsigned char trKelime[20], char ingKelime[20], char aranan[20], int secim, int *yer)
{
	int index = 1;
	*yer = 0;
	
	switch(secim)
	{
		case 1 : // Turkce - Ingilizce Sozluk
			{	
				while(!feof(turkce3) || !feof(ingilizce3))
				{
					fscanf(turkce3, "%s", trKelime);
					fscanf(ingilizce3, "%s", ingKelime);
					
					if( strcasecmp(trKelime, aranan) == 0 )
					{
						break;
					}
					
					if(feof(turkce3))
					{
						return 0;
					}
					
					index++;
				}
				
				*yer = index;
				return 1;
				break;
			}
		case 2 : // Ingilizce - Turkce Sozluk
			{
				while(!feof(turkce3) || !feof(ingilizce3))
				{
					fscanf(turkce3, "%s", trKelime);
					fscanf(ingilizce3, "%s", ingKelime);
					
					if( strcasecmp(ingKelime, aranan) == 0 )
					{
						break;
					}
					
					if(feof(ingilizce3))
					{
						return 0;
					}
					
					index++;
				}
				
				*yer = index;
				return 1;
				break;
			}
		default :
			{
				return 0;
				break;
			}
	}
}
