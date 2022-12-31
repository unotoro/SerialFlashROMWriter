//SDメモリからシリアルフラッシュに、バイナリデータを書き込みます
//ベリファイチェック付き
//ファイル名は158行目
//SROMサイズは145行目

/*
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 10 (for MKRZero SD: SDCARD_SS_PIN)

 ** CS - pin 9 (シリアルROMのCSpin)
*/

#include <SPI.h>
#include <SD.h>
const int chipSelect = 10;

#include <string.h>
#include <W25Q128.h>

//
// 書込みデータのダンプリスト
// dt(in) : データ格納先頭アドレス
// n(in)  : 表示データ数
//
byte dump(byte *dt, uint32_t n , uint32_t ofst) {
  char buf[64];
  uint16_t clm = 0;
  byte data;
  byte sum;
  byte vsum[16];
  byte total =0;
  uint32_t saddr =0;
  uint32_t eaddr =n-1;
  
  Serial.println("----------------------------------------------------------");
  for (uint16_t i=0;i<16;i++) vsum[i]=0;  
  for (uint32_t addr = saddr; addr <= eaddr; addr++) {
    data = dt[addr];
    if (clm == 0) {
      sum =0;
      sprintf(buf,"%06lx: ",(addr + ofst));
      Serial.print(buf);
    }

    sum+=data;
    vsum[addr % 16]+=data;
    
    sprintf(buf,"%02x ",data);
    Serial.print(buf);
    clm++;
    if (clm == 16) {
      sprintf(buf,"|%02x ",sum);
      Serial.print(buf);      
      Serial.println("");
      clm = 0;
    }    
  }
  Serial.println("----------------------------------------------------------");
  Serial.print("        ");
  for (uint16_t i=0; i<16;i++) {
    total+=vsum[i];
    sprintf(buf,"%02x ",vsum[i]);
    Serial.print(buf);
  }
  sprintf(buf,"|%02x ",total);
  Serial.print(buf);      
  Serial.println("");
  return total;
}



// Subroutine for writing data in SD card, SDカードへのデータ書き込みのためのサブルーチン
bool PrintToFile(byte *dataIn,String file_name){
  bool temp =false;
  File dataFile = SD.open(file_name, FILE_WRITE);  // define the filename, ファイル名を定義。
  if (dataFile) {                                      //if the file in the SD card was open to wrihte, SDカードの対象ファイルを開くことができれば
    dataFile.write(dataIn , 256);                          // write data into the file, データの記入 
    dataFile.close();                                  // close the file, ファイルを閉じる
    temp = true;                                       //1は成功
  }
  return(temp);
}



void setup() {
    byte buf[256];        // 取得データ
    byte wdata[16];       // 書込みデータ
    uint16_t n;           // 取得データ数
    
    W25Q128_begin(9,10000);     // フラッシュメモリ利用開始 csのpin番,転送clk
    Serial.begin(115200);
    
    // JEDEC IDの取得テスト
    W25Q128_readManufacturer(buf);
    Serial.print("JEDEC ID : ");
    for (byte i=0; i< 3; i++) {
      Serial.print(buf[i],HEX);
      Serial.print(" ");
    }
    Serial.println("");
    
    // Unique IDの取得テスト
    memset(buf,0,8);
    W25Q128_readUniqieID(buf);
    Serial.print("Unique ID : ");
    for (byte i=0; i< 8; i++) {
      Serial.print(buf[i],HEX);
      Serial.print(" ");
    }
    Serial.println("");




    // ステータスレジスタ1の取得
    buf[0] = W25Q128_readStatusReg1();
    Serial.print("Status Register-1: ");
    Serial.print(buf[0],BIN);
    Serial.println("");

    // ステータスレジスタ2の取得
    buf[0] = W25Q128_readStatusReg2();
    Serial.print("Status Register-2: ");
    Serial.print(buf[0],BIN);
    Serial.println("");



//-------------------------------------------------------------
// S-ROMを読み取ってsdに書き込む
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");

//4MBは16384、8MBは32768、16MBは65536まで
//                      ↓
  for (uint32_t i=0;i<16384;i++){                  
           
    // データの読み込み(アドレス0から256バイト取得)
    memset(buf,0,256);
    //n =  W25Q128_fastread(i*256,buf, 256);
    n =  W25Q128_read(i*256,buf, 256);
    Serial.print("Addres=");
    Serial.print(i,HEX);
    Serial.print("xxh  ");
    Serial.print("Read Data: n=");
    Serial.println(n,DEC);
    dump(buf,256 ,  i*256);
  
    PrintToFile(buf,"20220102.bin");//読み出しファイル名
  }
}



void loop() {

}
