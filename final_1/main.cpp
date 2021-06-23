#include "mbed.h"
#include "bbcar.h"
#include "bbcar_rpc.h"
#define PI 3.14159265
// RPC
Ticker servo_ticker;
PwmOut pin5(D11), pin6(D13);
BufferedSerial xbee(D10, D9);

BBCar car(pin5, pin6, servo_ticker);

void RPC_reverse(Arguments *in, Reply *out);
RPCFunction rpcRE(&RPC_reverse, "reverse");
// openmv
BufferedSerial pc(USBTX,USBRX); //tx,rx
BufferedSerial uart(D1,D0); //tx,rx
InterruptIn btnRecord(USER_BUTTON);

Thread thread1;

const int MaxBufferSize = 32;
char buffer[MaxBufferSize];
char oprec[1];
int  len = 0;
int angle = 180;
int d = 40;
bool record = true;

void command()
{
   while (record)
   {
      if(uart.readable())
      {
         for (int s=0; s < 3; s++)
         {
            uart.read(oprec,sizeof(oprec));
            buffer[len++] = oprec[0];
            buffer[len] = '\0';
         }
         sscanf(buffer, "%d", &angle);
         if (oprec[0] == '\n') {
                len = 0;
            }
      }
   }
   if(angle < 170)
   {
      int dist = tan((180-angle)*PI/180)*30;
      int move = 80 - dist;
      car.goStraight(50);
      ThisThread::sleep_for(move*100ms);
      car.stop();

      car.turn(47,1);
      ThisThread::sleep_for((angle-90)*10);
      car.stop();

      move = sqrt(30*30+dist*dist); //Pythagorean theorem
      car.goStraight(50);
      ThisThread::sleep_for((move)*100);
      car.stop();

      car.turn(50,-1);
      ThisThread::sleep_for((angle-90)*10);
      car.stop();

      car.goStraight(50);
      ThisThread::sleep_for(3s);
      car.stop();
   }

   if(angle > 190)
   {
      int dist = tan((angle-180)*PI/180)*30;
      int move = 80-dist;
      car.goStraight(50);
      ThisThread::sleep_for(move*100);
      car.stop();

      car.turn(47,-1);
      ThisThread::sleep_for((270-angle)*10);
      car.stop();

      move = sqrt(30*30+dist*dist); //Pythagorean theorem
      car.goStraight(50);
      ThisThread::sleep_for((move)*100);
      car.stop();

      car.turn(50,1);
      ThisThread::sleep_for((270-angle)*10ms);
      car.stop();

      car.goStraight(50);
      ThisThread::sleep_for(3s);
      car.stop();
   }
   
}

void steer()
{
   record = false;
}

void RPC_reverse(Arguments *in, Reply *out)
{
   if (angle < 180)
   {
      car.turn(47,-1);
      ThisThread::sleep_for(2500ms);
      car.stop();
      car.goStraight(50);
      ThisThread::sleep_for(11s);
      car.stop();
      car.turn(47,-1);
      ThisThread::sleep_for(1s);
      car.stop();
      car.goStraight(50);
      ThisThread::sleep_for(3s);
      car.stop();
      car.turn(47,-1);
      ThisThread::sleep_for(1s);
      car.stop();
   }
   else 
   {
      car.turn(47,1);
      ThisThread::sleep_for(2300ms);
      car.stop();
      ThisThread::sleep_for(500ms);
      car.goStraight(50);
      ThisThread::sleep_for(10s);
      car.stop();
      ThisThread::sleep_for(500ms);
      car.turn(47,-1);
      ThisThread::sleep_for(1s);
      car.stop();

      car.goStraight(50);
      ThisThread::sleep_for(3s);
      car.stop();

      car.turn(47,-1);
      ThisThread::sleep_for(1s);
      car.stop();
   }
}

int main() {
   char buf[256], outbuf[256];
   FILE *devin = fdopen(&xbee, "r");
   FILE *devout = fdopen(&xbee, "w");

   uart.set_baud(9600);
   thread1.start(command);
   btnRecord.rise(steer);


   while (1) {
      memset(buf, 0, 256);
      for( int i = 0; ; i++ ) {
         char recv = fgetc(devin);
         if(recv == '\n') {
            printf("\r\n");
            break;
         }
         buf[i] = fputc(recv, devout);
      }
   RPC::call(buf, outbuf);
   }
}