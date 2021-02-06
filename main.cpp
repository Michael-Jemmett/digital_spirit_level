#include "mbed.h"
#include "TextLCD.h"
#include "ADXL345.h" 
InterruptIn Button(p10);
DigitalOut myled(LED1);
TextLCD lcd(p15, p16, p17, p18, p19, p20); // rs, e, d4-d7
ADXL345 accelerometer(p11, p12, p13,p14); // SDA, SDO, SCL, CS
Serial pc(USBTX, USBRX); 
float Pitch,Roll,Pitch_cali,Roll_cali;
int flagcheck;

void ISR(){                 //Interrupt routine to set the current pitch and roll values as the new 0 values
    Roll_cali = Roll;
    Pitch_cali = Pitch;
    flagcheck = 1;  
    }

int main(void){
    Button.fall(&ISR);
    float Roll_change,Pitch_change,x = 0,y = 0,z = 0;
    int readings[3] = {0, 0, 0};
    int x_total,y_total,z_total;
    int x_filter[10], y_filter[10], z_filter[10];
    accelerometer.setPowerControl(0x00);            //set up of the accelerometer
    accelerometer.setDataFormatControl(0x0B);  
    accelerometer.setDataRate(ADXL345_3200HZ); 
    accelerometer.setPowerControl(0x08); 
    
    while(1){
        if(flagcheck == 1){      //If the interrupt was called then clear the flag
            flagcheck = 0;
            myled = 0; 
        }
        int i=0;
        while(i!=10){
            accelerometer.getOutput(readings);  //reads the accelerometer to get the x y & z values
        
            x_filter[i] = (int16_t)readings[0];
            y_filter[i] = (int16_t)readings[1];
            z_filter[i] = (int16_t)readings[2];
            i++;
        }
      
        for(int j=1; j<4; j++){     //applies a 3 point moving average filter to all vlaues
            x_total = ((x_filter[j] + x_filter[j+1] + x_filter[j+2] )/3);
            y_total = ((y_filter[j] + y_filter[j+1] + y_filter[j+2] )/3);
            z_total = ((z_filter[j] + z_filter[j+1] + z_filter[j+2] )/3);
        
        x = x + x_total;
        y = y + y_total;
        z = z + z_total;
        }
        x = (x/3); //have to divide by 3 to obtain the true average
        y = (y/3);
        z = (z/3);
            
        pc.printf("x total is %.2f \n \r", x);
        pc.printf("y total is %.2f \n \r", y);
        pc.printf("z total is %.2f \n \r", z);
        pc.printf(" \n \r");
                    
        wait(1);
        Roll = atan2(-y,z)*180/3.14;                //roll angle calculation in degress
        Pitch = atan2(x,sqrt(y*y+z*z))*180/3.14;    //pitch angle calculation in degress
           
        Pitch_change = Pitch - Pitch_cali;
        Roll_change = Roll - Roll_cali; 
        lcd.cls();
        lcd.printf("Pitch: %.2F\n",Pitch_change);   //Y angle
        lcd.printf("Roll: %.2F",Roll_change);       //X angle
      
        x = 0;                                      //clears all of saved values 
        x_total = 0;
        y = 0;
        y_total = 0;
        z = 0;
        z_total = 0;
        if(Pitch_change > 45 ||  Roll_change > 45){ //turns LED on if the pitch or roll > 45 degrees
            myled = 1;
        }
    }
}