#include <iostream>
#include <fstream>

using namespace std;

void read_hour(istream& ins , int& hour);
void read_minute(istream& outs,int& mins);
int digit_to_int(char c);
class DigitalTime{
    public:
        friend int operator == (const DigitalTime& time1,const DigitalTime& time2);
        DigitalTime(int hours,int min);
        DigitalTime();
        void advance(int min_add);
        void advance(int hour_add , int min_add);
        friend istream& operator >> (istream& ins , DigitalTime& object);
        friend ostream& operator << (ostream& outs,DigitalTime& object);

    private:
        int hour;
        int minutes;

};


int main (){
    DigitalTime clock,clocko;
    cout<<"Enter the time in 24 hoour system:";
    cin>>clocko;

    clock=clocko;
    clock.advance(15);
    if (clock==clocko){
        cout<<"Something wrong";
    }
    cout<<"You entered"<<clocko<<endl;
    cout<<"now is "<<clock;

    return 0;
    
}


int operator == (const DigitalTime& time1,const DigitalTime& time2){
    return (time1.hour==time2.hour && time1.minutes==time2.minutes);
}

DigitalTime::DigitalTime(){
    hour=0;
    minutes=0;
}

DigitalTime::DigitalTime(int hours,int min){
    if (hour>=24||min>=60||hour<0||min<0){
        cout<<"Error hour and minute information"<<endl;
        exit(0);
    }
    else{
        hour=hours;
        minutes=min;
    }}
void DigitalTime::advance(int min_add){
    int gross_min=minutes+min_add;
    minutes=gross_min%60;

    hour=(hour+gross_min/60)%24;

}

void DigitalTime::advance(int hour_add , int min_add){
    hour=hour+hour_add;
    advance(min_add);
}


istream& operator >> (istream& ins, DigitalTime& object){
    read_hour(ins,object.hour);
    read_minute(ins,object.minutes);
    return ins;
}


ostream& operator << (ostream& outs,DigitalTime& object){
    outs<<object.hour<<".";
    if (object.minutes<10){
        outs<<"0";

    }
    outs<<object.minutes;
    return outs;

}


int digit_to_int (char c){
    return (int(c)-int('0'));
}

void read_hour(istream& ins , int& the_hour){
    char c1,c2;
    ins>>c1>>c2;

    if (isdigit(c1)&&c2==':'){
        the_hour=digit_to_int(c1);
    }
}

void read_minute(istream& ins , int& the_minutes){
    char c1,c2;
    ins>>c1>>c2;
    the_minutes=digit_to_int(c1)*10+digit_to_int(c2);
}
