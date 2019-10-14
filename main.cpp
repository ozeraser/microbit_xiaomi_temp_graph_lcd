#include "MicroBitCustomConfig.h"
#include "MicroBit.h"
#include "LCD_Driver.h"
#include "ble/DiscoveredCharacteristic.h"
#include "ble/DiscoveredService.h"

#if YOTTA_CFG_MICROBIT_S130 != 1
#error This code *only* works with the Nordic S130 softdevice
#endif

#if CONFIG_ENABLED(MICROBIT_DBG)
#error use of the serial port by MICROBIT_DBG clashes with our use of the serial port - not uspported
#endif

MicroBit uBit;
LCD_Driver ldriv;
char buf[10];
char buf2[10];
char tempbuf1[8];
char tempbuf2[8];
char tempbuf3[30];
char tempbuf4[8];
char microseconds[50];
int temp=-1000;
int tempoffset=9; // microbit shows 10C difference to the ambiant temp
int tempaveragea=0; // accumulation of temperature readings
int tempsamplesa=0; //how many readings
int tempaverageb=0; // accumulation of temperature readings (board temp))
int tempsamplesb=0; //how many readings (boardtemp)
int hum=-1;
int bat=-1;
int x,cnt=0,mode=0,v1,v10;
int tempOld=-123;
int humOld=-123;
int batOld=-123;
int cntOld = -1;
int Lineloc = 26;
int t = 0;
unsigned long tmT=0;
unsigned long tmH=0;
unsigned long tmB=0;
unsigned long tmD=0;
char *modeTxt="";

// Xiaomi advertisement packet decoding
//          18       21 22 23 24
//          mm       tl th hl hh 
// a8 65 4c 0d 10 04 da 00 de 01  -> temperature+humidity
//          mm       hl hh
// a8 65 4c 06 10 04 da 01        -> humidity
//          mm       tl th
// a8 65 4c 04 10 04 db 00        -> temperature
//          mm       bb 
// a8 75 4c 0a 10 01 60           -> battery
// 75 e7 f7 e5 bf 23 e3 20 0d 00  -> ???
//          21 e6 f6 18 dc c6 01  -> ???
// b8 65 5c 0e 10 41 60           -> battery??
// a8 65 4c 46 10 02 d4 01        -> ?? 
//b9 ff
//bf ff

//d0 ff -4
//fe ff -0.6
//3b 00 5.9

int fntempconv(char *var1, char *var2) 
{
//fntempconv(params->advertisingData[22],params->advertisingData[21]);
//fntempconv("0xff","0xd0"); -4.8
//fntempconv("0xff","0xd0"); -4.8
//fntempconv("0xff","0xfe"); -0.2
//fntempconv("0x00","0x3b"); -5.9
//fntempconv("0x00","0x6d"); -10.9

    int msb = strtol(var1, NULL, 0);
    int lsb = strtol(var2, NULL, 0);

    int value = (((msb & 0xff) << 8) | (lsb & 0xff));
    if (value >= 0x8000) value = -(0x10000 - value);
//printf("value %.1f\n",percentage);
    return value;
}

// this is the graphing function
// It takes the tempval, the id of the value (incase we have multiple temps)
void fngraph(int tempval, int idtemp) 
{
//TODO show differnt BT thingos if needed
tempaveragea = tempaveragea + tempval; //add current temp
tempsamplesa= tempsamplesa+1; //how many readings

tempaverageb = tempaverageb + (uBit.thermometer.getTemperature()-tempoffset); //add current temp
tempsamplesb= tempsamplesb+1; //how many readings

if (uBit.systemTime() - t > 600000) {
    t = uBit.systemTime();
    ldriv.LCD_DrawLine(Lineloc,80,Lineloc,24,0x0000); //clear positive
    ldriv.LCD_DrawLine(Lineloc,80,Lineloc,112,0x0000); //clear negative
    tempval = tempaverageb / tempsamplesb; //average microbt temp
    ldriv.LCD_DrawLine(Lineloc,80,Lineloc,80-tempval,0x03E0); //TBD make 80 or middle line a constant and detect negative MANUALLY FIRST AS EXPECTED HIGHER, function needed to detect
    tempval = tempaveragea / tempsamplesa; //overwrite actual temp
    ldriv.LCD_DrawLine(Lineloc,80,Lineloc,80-tempval,0xF800); //TBD make 80 or middle line a constant and detect negative
    tempaveragea = 0;
    tempsamplesa = 0;
    tempaverageb = 0;
    tempsamplesb = 0;
    if (Lineloc < 159) {
      Lineloc++;
      ldriv.LCD_DrawLine(Lineloc,80,Lineloc,24,0xFFE0); //clear positive
      ldriv.LCD_DrawLine(Lineloc,80,Lineloc,112,0xFFE0); //clear negative
    }else {
      Lineloc = 26;
      ldriv.LCD_DrawLine(Lineloc,80,Lineloc,24,0xFFE0); //clear positive
      ldriv.LCD_DrawLine(Lineloc,80,Lineloc,112,0xFFE0); //clear negative
    }
}
}

void advertisementCallback(const Gap::AdvertisementCallbackParams_t *params)
{
    int plSize=params->advertisingDataLen;

  if(params->advertisingData[5]==0x95 && params->advertisingData[6]==0xfe && params->advertisingData[7]==0x50 && params->advertisingData[8]==0x20 && plSize>22)
  {
    cnt=params->advertisingData[11];
    mode=params->advertisingData[18];
    int mesSize=params->advertisingData[3];
    if(mode==0x0d && plSize==25) { // temperature + humidity (missing msb, lsb is reconstructed from previous value)
      temp=params->advertisingData[21]+params->advertisingData[22]*256;
      snprintf(tempbuf1,8,"0x%02x",params->advertisingData[21]);
      snprintf(tempbuf2,8,"0x%02x",params->advertisingData[22]);
      snprintf(microseconds,50,"%d",uBit.systemTime());
      int answer = fntempconv(tempbuf2,tempbuf1);
      fngraph (answer / 10,1);
      snprintf(tempbuf3,8,"%d",answer);
      snprintf(tempbuf4,8,"%d",uBit.thermometer.getTemperature()-tempoffset);
      //ldriv.LCD_Clear(0xFFFF);
      ldriv.LCD_DrawRectangle(1, 1, 120, 12,0x0000, 1); //clear existing text
      //ldriv.LCD_DisString(60,45,tempbuf1,0xFFFF);
      //ldriv.LCD_DisString(60,55,tempbuf2,0xFFFF);
      ldriv.LCD_DisString(1,1,tempbuf3,0xFFFF);
      ldriv.LCD_DisString(30,1,tempbuf4,0xFFFF);
      ldriv.LCD_DisString(60,1,microseconds,0xFFFF);
      ldriv.LCD_Display();

      //uBit.display.scrollAsync(tempbuf3);
          //uBit.display.scrollAsync("f" + params->advertisingData[21] + "s" + ManagedString(params->advertisingData[22]*256) + "|" );
      hum=params->advertisingData[23]+params->advertisingData[24]*256;;
      modeTxt="TH";
      //tmT = millis();
    } else if(mode==0x04 && plSize==23) {  // temperature 
      temp=params->advertisingData[21]+params->advertisingData[22]*256;
      modeTxt="T ";
      //if(sdebug) snprintf(buf,100,"#%02x %02x %s %02x %3d'C       ",cnt,mode,modeTxt,recv[3],recv[21]+recv[22]*256);
      //tmT = millis();
    } else if(mode==0x06 && plSize==23) {  // humidity
      hum=params->advertisingData[21]+params->advertisingData[22]*256;
      modeTxt="H ";
      //if(sdebug) snprintf(buf,100,"#%02x %02x %s %02x %3d%%        ",cnt,mode,modeTxt,recv[3],recv[21]+recv[22]*256);
     // tmH = millis();
    } else if(mode==0x0a && plSize==22) {  // battery level
      bat=params->advertisingData[21];
      modeTxt="B ";
      //if(sdebug) snprintf(buf,100,"#%02x %02x %s %02x %03d%% batt   ",cnt,mode,modeTxt,recv[3],recv[21]);
      //tmB = millis();
    } else {
      modeTxt="??";
      //if(sdebug) snprintf(buf,100,"!!!!!!%02x %02x %s %02x %03d %03d",cnt,mode,modeTxt,recv[3],recv[21],recv[22]);
    }
    if(tempOld==temp && humOld==hum && batOld==bat) return;
    tempOld=temp; humOld=hum; batOld=bat;
// every 12 minutes lets post.

    
    //65444
    //65499 (-3.2)
    //65510 (-2)
    //65521 (-1)
    //65532
    
//f109
//f237-255
//f247-255
//f248-255
//f247-255
//f248-255
//f249-255
//f255-255 0.2
//f12-0  01.2C
//f0.1


  if(temp<=-400 || temp>800)
     strcpy(buf,"--.- ");
  else {
    v1=temp/10;
    v10=temp-v1*10;
    snprintf(buf,10,"%2d.%d ",v1,v10);
  }
  if(hum<0 || hum>1000)
     strcpy(buf2,"--.- ");
  else {
    v1=hum/10;
    v10=hum-v1*10;
    snprintf(buf2,10,"%2d.%d ",v1,v10);
  }
  
    //uBit.display.scrollAsync(" t " + ManagedString(buf) + " h " + ManagedString(buf2) + " t " + modeTxt);
  }

    
    
    
    
    
    
    
    
    
    
    
            char uuid[33]{};
            uint8_t unib{};
            uint8_t lnib{};

            for (int i = 0; i < 16; ++i)
            {
                unib = params->advertisingData[6 + i] >> 4;
                lnib = params->advertisingData[6 + i] & 0xf;

                if (unib < 10)
                {
                    uuid[i * 2] = '0' + unib;
                }
                else
                {
                    uuid[i * 2] = 'A' + unib - 10;
                }
                if (lnib < 10)
                {
                    uuid[i * 2 + 1] = '0' + lnib;
                }
                else
                {
                    uuid[i * 2 + 1] = 'A' + lnib - 10;
                }
            }

           // uBit.display.scrollAsync(uuid);

}

int main()
{
long t;
ldriv.LCD_Init();
ldriv.LCD_SetBL(10);
ldriv.LCD_ClearBuf();
ldriv.LCD_Clear(0x0000);
//ldriv.LCD_DrawPoint(30, 30, 0x001F, 3);

//Y axis 112 - 80 This is the negative area (22 pixels) ~0 >> -22 Degrees(dont need -20 really, but what if it gets to -20)
//Y axis 80 - 20 This is the positive area (60 pisels) ~60 >> 0 Degrees C (Might scale this)
//X axis 20 - 150 This is the time area (144 pixels) 1440 minutes in a day, ~10 minues per pixel which is 600,000 milliseconds 
ldriv.LCD_DrawLine(25,24,24,112,0x001F);
ldriv.LCD_DrawLine(25,80,159,80,0x001F);
ldriv.LCD_DisString(20,116,"Time 1 Bar - 10min",0X8430);
ldriv.LCD_DisString(1,31,"T",0X8430);
ldriv.LCD_DisString(1,42,"e",0X8430);
ldriv.LCD_DisString(1,53,"m",0X8430);
ldriv.LCD_DisString(1,64,"p",0X8430);
ldriv.LCD_DisString(1,81,"C",0X8430);
ldriv.LCD_DisString(10,74,"0",0xFFFF);
ldriv.LCD_DisString(1,105,"-20",0xFFFF);
ldriv.LCD_DisString(5,20,"40",0xFFFF);
ldriv.LCD_Display();
//ldriv.LCD_DrawCircle(30,40,10,0x001F,0);
//ldriv.LCD_DrawCircle(60,45,10,0x001F,1);
t = uBit.systemTime(); //remember the start timer, use this later to figure out when 10 minutes is up.
Lineloc = 26; // insure we start at the first X

    uBit.ble = new BLEDevice();
    uBit.ble->init();

    uBit.ble->gap().setScanParams(500, 400);
    uBit.ble->gap().startScan(advertisementCallback);

    while (true)
    {
        uBit.ble->waitForEvent();
    }
    return 0;
}