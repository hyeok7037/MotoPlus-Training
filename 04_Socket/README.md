# 03_SVar_TCP

C# 클라이언트에서 ASCII 문자열을 전송하고 MotoPlus TCP 서버가 수신한 문자열을 `S010`에 기록하는 예제입니다.

```text
04_Socket/
└── 03_SVar_TCP/
    ├── MotoPlus/
    │   └── mpMain.c
    ├── CSharp/
    └── README.md
```

통신 형식은 `ASCII 문자열 + LF(\n)`입니다. `STR_VAR_SIZE == 33`이면 실제 문자열은 최대 32바이트이며 index 32에 NULL이 저장됩니다.
