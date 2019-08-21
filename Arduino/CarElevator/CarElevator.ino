#include <SoftwareSerial.h>
#include <EEPROM.h>

const int BLUETOOTH_TX = 3;     // HC-06(블루투스 모듈) Tx핀
const int BLUETOOTH_RX = 2;     // HC-06(블루투스 모듈) Rx핀

const int ECHO = 5;             // HC-SR04(초음파 센서) Echo핀
const int TRIG = 6;             // HC-SR04(초음파 센서) Trig핀

const int MORTOR_A = 10;         // L298N(모터 드라이브) In1 핀
const int MORTOR_B = 11;         // L298N(모터 드라이브) In2 핀
const int MORTOR_PWM = 9;      // L298N(모터 드라이브) ENA 핀

const int PWM = 130;            // 모터의 출력속도
const int MORTOR_DELAY = 1470;  // 주차장 한칸이동시 모터의 작동 시간

const int CARINDEX_MAX = 8;     // 적재할 수 있는 차량의 최대 수
const int CAR_DISTANCE = 40;   // 초음파 센서에서 차량유무 인식하는 거리

boolean enable = false;           // 현재 해당 장치가 사용 가능한지 여부
int carIndex[CARINDEX_MAX];         // 각 차고별 차량 적재 여부를 담고있는 배열
int moveDelay[CARINDEX_MAX];        // 현재위치에서 각 차고별 이동거리를 담고 있는 배열
int currCar = 0;                    // 현재 입구의 위치
boolean firstFlag = false;
int count_Time = 0;
int check_Time = 0;
 
SoftwareSerial bluetooth(BLUETOOTH_TX,BLUETOOTH_RX);     //블루트스 객체 생성 

/* 모터를 특정 방향으로 작동시킴
 * @prams dir 모터의 방향
 */

int Mortor(int dir);        // 모터 제어 함수
void wait();                // 차량이 없을떄 까지 기다림
void moveEnter(int mov);    // 입구를 mov위치로 이동
void getDelay();            // 차고의 차량 유무 검색후 각 차고의 거리를 계산
int nextCar();              // 가장 최단거리에 있는 층을 출력
void InserCar();            // 차량을 입고함
void OutCar(int index);     // 차량을 출고함
int CheckMicroWave();       // 초음파 센서 거리계산
int CheckSize();            // 빈 차고 수 출력
void showData();            // 배열에 담겨있는 데이터를 Serial에 출력

int CheckMicroWave();

int Mortor(int dir){
  if(dir == 1){                   //dir 이 1일시 정방향 작동
    digitalWrite(MORTOR_A, HIGH);
    digitalWrite(MORTOR_B, LOW);
    analogWrite(MORTOR_PWM, PWM);
    delay(MORTOR_DELAY);
    digitalWrite(MORTOR_A, LOW);
    analogWrite(MORTOR_PWM, 0);
    if(currCar == CARINDEX_MAX){
      currCar = 0;
      EEPROM.write(0,currCar);
    }else{
      currCar = currCar + 1;
      EEPROM.write(0,currCar);
    }
  }else if(dir == 0){             //dir 이 0일시 역방향 작동
    digitalWrite(MORTOR_B, HIGH);
    digitalWrite(MORTOR_A, LOW);
    analogWrite(MORTOR_PWM, PWM);
    delay(MORTOR_DELAY);
    analogWrite(MORTOR_B, LOW);
    analogWrite(MORTOR_PWM, 0);
    if(currCar == 0){
      currCar = CARINDEX_MAX;
      EEPROM.write(0,currCar);
    }else{
      currCar = currCar - 1;
      EEPROM.write(0,currCar);
    } 
  }
  return currCar;
}

//원하는 목적 층으로 이동
void moveEnter(int mov){
  Serial.print(currCar);
  Serial.print("에서 ");
  Serial.print(mov);
  Serial.println("이동");

  int moveNum = mov - currCar;
  int dir;
  String str = ""; 
  
  if(moveNum < 0){
    dir = 0;
    str = "아래로";
  }else {
    dir = 1;
    str = "위로";
  }
    
  
  for(int i = 0; i < abs(moveNum)+1; i++){
    if(currCar == mov){
      Serial.print(mov);
      Serial.println("에 도착했습니다.");
    }else {
      Serial.print(currCar);
      Serial.print("에서");
      Serial.print(str);
      Serial.println("이동합니다.");
      Mortor(dir);
    }
  }
}

//  거리 검사
void getDelay(){
  for(int i = 0; i< CARINDEX_MAX; i++){
    if(carIndex[i] == 0){
      int time_d = i - currCar;
       moveDelay[i] = abs(time_d);
    }else {
      moveDelay[i] = -1;
    }
  }
  showData();
}


// 가장 가까운 적재 장소 구하기
int nextCar(){
  getDelay();

  int result = -1;
  int index;
  int temp_MIN = CARINDEX_MAX + 1;
  boolean ena = false;
  
  for(int i = 0; i < CARINDEX_MAX; i++){
    if(moveDelay[i] != -1){
      if(moveDelay[i] < temp_MIN){
        index = i;
        temp_MIN = moveDelay[i];
        ena = true;
      }
    }
  }

  if(ena == false)
    result = -1;
  else
    result = index;
  Serial.println("---다음---");
  Serial.println(index);
  Serial.println("----------");
  return result;
}

// 차량 삽입하기

void InserCar(){
    carIndex[currCar] = 1;
    if(InserCar == 7){
       moveEnter(0);
    }else {
       firstFlag = false;
      if(CheckSize != 0){
      int next = nextCar();
      moveEnter(next);
      Serial.print(currCar);
      Serial.println("에 입고합니다.");
      bluetooth.print("%");
      bluetooth.print("n");
      bluetooth.print((currCar));
      bluetooth.print("?");
     }else {
        bluetooth.print("%");
        bluetooth.print("k");    
        bluetooth.print("?");
    }
    }
}

void OutCar(int index){
  Serial.println("---------");
  Serial.print("Index :");
  Serial.print(index);
  Serial.println("---------");
  if(carIndex[index] == 1){
    moveEnter(index);
    carIndex[index] = 0;
    Serial.print(index);
    Serial.println("를 출고 합니다.");
    bluetooth.print("%");
    bluetooth.print("s");
    bluetooth.print(index);
    bluetooth.print("?");
  }else {
    Serial.println("차량이 없습니다.");
    bluetooth.print("%");
    bluetooth.print("f");
    bluetooth.print("?");
  }
  wait();
  showData();
}

// 초음파 센서를 측정
int CheckMicroWave(){
    //초음파센서 작동
  digitalWrite(TRIG, LOW);
  digitalWrite(ECHO, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  // ECHO 이 HIGH를 유지한 시간을 저장 한다.
    unsigned long duration = pulseIn(ECHO, HIGH); 
  // HIGH 였을 때 시간(초음파가 보냈다가 다시 들어온 시간)을 가지고 거리를 계산 한다.
  float distance = ((float)(340 * duration) / 10000) / 2;
 // Serial.println("초음파 거리");
 // Serial.println("\n");  
 // Serial.print(distance);
  return distance;
}

void wait(){
  boolean waitFlag = true;
  int distance = CheckMicroWave();
  Serial.print("차량나감 대기중");
  while(waitFlag){
    if(distance < CAR_DISTANCE ){
      delay(100);
      Serial.print(".");
    }else {
      waitFlag = false;
      Serial.println("\n차량이 나감");
      delay(1000);
    }
  }
  
}

int CheckSize(){
  showData();
  int count = 0;
  for(int i = 0; i < CARINDEX_MAX; i++){
    if(carIndex[i] == 1){
      count = count + 1;
    }
  }
  Serial.print("count : ");
  Serial.println(count);
  return CARINDEX_MAX - count;
}

String printList(){
  showData();
  String str = "%";
  for(int i = 0; i < CARINDEX_MAX; i++){
    str = str +  String(carIndex[i]);
  }
  str = "?";
  Serial.print(str);
  return str;
}

void showData(){
  Serial.println("---------------");
  for(int i = 0; i< CARINDEX_MAX; i++){
    Serial.print(i);
    Serial.print("번쨰 값 : ");
    Serial.print(carIndex[i]);
    Serial.print(" 거리 : ");
    if(currCar == i){
      Serial.print(moveDelay[i]);
      Serial.println("<");
    }else {
      Serial.println(moveDelay[i]);
    }
  }
  Serial.println("---------------");
}
 
void setup()
{

  // 1번 차고는 입구로 사용 되기에 초기값을 들어있음으로 선언
  carIndex[0] = 1;
  carIndex[1] = 1;
  carIndex[2] = 1;
  carIndex[3] = 0;
  carIndex[4] = 1;
  carIndex[5] = 1;
  carIndex[6] = 0;
  carIndex[7] = 1;
  Serial.begin(9600);//시리얼 통신 초기화
  bluetooth.begin(9600);//블루투스 통신 초기화
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  delay(1000);
  //EEPROM.write(0,0);
  currCar = EEPROM.read(0);
  
}



void loop()
{ 
  
  // 초음파 센서 거리측정값 받아옴
  int distance = CheckMicroWave();
  
  // 해당 지역에 차있는지 없는지 확인
  if(count_Time  > check_Time){
     if(distance < CAR_DISTANCE ){
    if(firstFlag){
        firstFlag = false;
    }else {
       firstFlag = true;
       check_Time = count_Time + 2000;
        Serial.println("\n-----------");
        Serial.println("Inner Car");
        Serial.println("-----------");
    }
  }
}
 
  

  


  //블루투스에서 읽은 문자를 시리얼로 전송
  if(bluetooth.available())
  {
    char toSend = bluetooth.read();
    Serial.println(toSend);
    if(toSend == 'i')
      if(firstFlag){
        InserCar();
      }else {
        Serial.println("\n-----------");
        Serial.println("No Car");
        Serial.println("-----------");
      }
      
    else if(toSend == 'o'){
      int count = CheckSize();
      bluetooth.print("%");
      bluetooth.print("c");
      bluetooth.print(count);
      bluetooth.print("?");
    }else if(toSend == 'm'){
      bluetooth.print(printList());
      bluetooth.print("%");
      Serial.print(printList());
      bluetooth.print("?");
    }
    else{
      int index = -1;
      switch(toSend){
        case 49:
          index = 0;
          break;
        case 50:
          index = 1;
          break;
        case 51:
          index = 2;
          break;
        case 52:
          index = 3;
          break;
        case 53:
          index = 4;
          break;
        case 54:
          index = 5;
          break;
        case 55:
          index = 6;
          break;
        case 56:
          index = 7;
          break;

      }
        
      if(index != -1){
         OutCar(index);
      }
    }
  }

  delay(1);
  count_Time += 1;
}
