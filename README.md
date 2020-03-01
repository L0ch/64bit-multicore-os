
# README

### Requisites
Ubuntu 18.04.3 LTS 
binutils-2.30
gcc 5.5.0
nasm 2.13.02
eclipse 4.12.0

### Test Environment
QEMU 4.1.0-rc2 win32



**8/24**
부트로더 실행, OS 이미지 로딩

**8/26**
32bit 커널 전환

**9/4**
64bit 커널 전환

**10/1**
키보드 드라이버 추가 

#### 10/22
인터럽트 추가

![](https://i.imgur.com/dEGwe6m.png)

#### 11/6
각 인터럽트/예외 벡터의 핸들러 함수 추가
![](https://i.imgur.com/Pl8l7fc.png)

#### 11/20
키보드 드라이버 업그레이드(Queue 사용), 쉘/명령어 구현
![](https://i.imgur.com/PrOgNDr.png)
![](https://i.imgur.com/zezvYmB.png)

#### 12/9
타이머 디바이스 드라이버 추가/멀티태스킹 테스트 명령어 추가
![](https://i.imgur.com/PxwPBYS.png)


#### 2020/1/10

라운드 로빈 스케줄러 추가

멀티태스킹 테스트 명령어 수정 및 1000개 태스킹 실행 테스트
![](https://i.imgur.com/w2Oze9Q.gif)

#### 1/13

멀티레벨 큐 스케줄러 추가로 태스크 우선순위 설정/변경기능 추가

태스크 종료 및 프로세서 사용률 측정을 위한 유휴태스크 추가

**쉘 명령어 추가**
+ 태스크 리스트 출력
+ 태스크 종료 및 우선순위 변경 
+ 프로세서 사용률 출력

![](https://i.imgur.com/XP2FHzD.png)

![](https://i.imgur.com/IflXy5x.png)

#### 1/16

태스크-인터럽트/태스크-태스크 간 동기화 처리 - 뮤텍스 동기화 객체 생성 및 임계영역 설정

**쉘 명령어 추가 및 수정**
+ 태스크 종료 - 쉘/유휴 태스크를 제외한 모든 태스크 종료기능 추가
+ 뮤텍스 테스트

![](https://i.imgur.com/hOqhgnr.png)
![](https://i.imgur.com/kbHQsaM.png)

#### 3/1
FPU 실수연산 기능

예외 #7 Device Not Available 핸들러 추가

Printf 실수표현 기능 추가

**쉘 명령어 추가**
+ 원주율 계산과 실수연산 테스트

![](https://i.imgur.com/0SAv9Ro.png)
