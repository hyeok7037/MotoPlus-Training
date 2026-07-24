# 03. IO

This chapter demonstrates how to read and write controller CIO data from a MotoPlus application.

The sample uses:

- `mpReadIO()`
- `mpWriteIO()`

> **Important**
>
> Review and change every CIO address before running the sample. Writing to an address used by the robot system, a job, a PLC, or another MotoPlus application can affect controller operation.

## 1. CIO and MotoPlus

MotoPlus does not access only physical input and output terminals. The IO APIs access the controller's CIO address space.

The CIO address space includes areas such as:

| CIO area | Address range |
|---|---:|
| Universal Input | `10` to `5127` |
| Universal Output | `10010` to `15127` |
| External Input | `20010` to `25127` |
| External Output | `30010` to `35127` |
| Specific Input | `40010` to `42567` |
| Specific Output | `50010` to `55127` |
| Interface Panel | `60010` to `60647` |
| Auxiliary Relay | `70010` to `79997` |
| Control Input | `80010` to `85127` |
| Pseudo Input | `87010` to `87207` |
| Network Input | `27010` to `29567` |
| Network Output | `37010` to `39567` |
| Register | `100000` to `100999` |

This chapter does not cover CIO ladder programming or CIO address calculation. It focuses only on reading and writing known CIO addresses from MotoPlus.

## 2. Reading CIO

### API

```c
LONG mpReadIO(MP_IO_INFO *sData, USHORT *rData, LONG num);
```

### Data structure

```c
typedef struct
{
    ULONG ulAddr;
} MP_IO_INFO;
```

- `sData` contains the CIO addresses to read.
- `rData` receives the CIO values.
- `num` is the number of addresses.
- Up to 253 addresses can be read in one call.

The addresses do not need to be consecutive. Each element of the `MP_IO_INFO` array specifies one CIO address.

### Example

```c
MP_IO_INFO ioInfo[3];
USHORT ioValue[3];

ioInfo[0].ulAddr = 37010;
ioInfo[1].ulAddr = 37011;
ioInfo[2].ulAddr = 37012;

mpReadIO(ioInfo, ioValue, 3);
```

The result is stored as follows:

```text
ioValue[0] <- CIO 37010
ioValue[1] <- CIO 37011
ioValue[2] <- CIO 37012
```

## 3. Writing CIO

### API

```c
LONG mpWriteIO(MP_IO_DATA *sData, LONG num);
```

### Data structure

```c
typedef struct
{
    ULONG ulAddr;
    ULONG ulValue;
} MP_IO_DATA;
```

Each element contains both the CIO address and the value to write.

According to the API manual, `mpWriteIO()` supports these writable areas:

| Writable CIO area | Address range |
|---|---:|
| Universal Output | `10010` to `15127` |
| Interface Panel | `60010` to `60647` |
| Network Input | `27010` to `29567` |
| Register | `100000` to `100559` |

### Example

```c
MP_IO_DATA ioData[3];

ioData[0].ulAddr = 27010;
ioData[0].ulValue = 1;

ioData[1].ulAddr = 27011;
ioData[1].ulValue = 0;

ioData[2].ulAddr = 27012;
ioData[2].ulValue = 1;

mpWriteIO(ioData, 3);
```

## 4. Sample flow

The included `mpMain.c` demonstrates four operations:

```text
Example 1: Read one CIO address
Example 2: Read multiple CIO addresses
Example 3: Write one CIO address
Example 4: Write multiple CIO addresses
```

The sample uses wrapper functions so the task code remains simple:

```c
BOOL ReadIO(ULONG ioAddr, USHORT *value);
BOOL ReadIOs(const ULONG *ioAddr, int count, USHORT *value);
BOOL WriteIO(ULONG ioAddr, ULONG value);
BOOL WriteIOs(const ULONG *ioAddr, const ULONG *value, int count);
```

## 5. Important notes

### Read timing

For YRC1000 and YRC1000micro, if a value changes faster than the IO control cycle, `mpReadIO()` may not capture a short pulse depending on task timing.

Use a pulse stretcher, latch signal, or another suitable signal design when short pulses must be detected reliably.

### Multiple writes

When multiple values are written at once, the controller temporarily raises the application task priority so the values are not changed by another task during the write operation.

Do not write an unnecessarily large number of CIO values while robot motion is running.

### Written value reflection

The written value may take time to be reflected. Do not assume that an immediate `mpReadIO()` or monitor read will always show the new value in the same instant.

### No automatic interlock

MotoPlus IO writes are not automatically interlocked with:

- Teach-mode pendant edits
- Job execution
- Other MotoPlus applications
- Concurrent CIO output changes

The application must prevent conflicting writes.
