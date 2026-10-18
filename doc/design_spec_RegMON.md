# Register Monitor

Author: Anders Bjoerk

## Revision

| Version | Date       | Changes               |
|:-------:|:----------:|:---------------------:|
| 0.1     | 2026/10/18 | Initial documentation |

[TOC]

## Module Context

In this chapter you can find the different context where a Register Monitor might be used.

### What is a Register Monitor

A Register Monitor is in its simple form, the access point for other components to access, mainly read and write, device registers.  

In embedded programming, and smaller devices, it is common to have a list of registers or parameters that can are globally accessed.  
By having a module monitoring the access to these, code can be made more safe and stable. Especially in devices that utilizes RTOS, where multiple task can access registers at once!

```plantuml
@startuml

component "Register Monitor" as regmon #orange
database "Registers" as regs
component "Modbus" as modbus
component "Controller" as CAN

regmon -- regs
modbus -- regmon
CAN -- regmon

Caption "Figure 1: Example of Register Monitor, as a component in a simple system"

@enduml
```

Register can be anything that the device need to operate, and should be made for each independent device.

## Module description

```plantuml
@startuml
left to right direction

actor "User Interface" as user
actor "Internal Interface" as if
rectangle "Register Monitor" as regmon{
    usecase "UC5: Read/Write protected data" as uc5
    usecase "UC4: Read/Write Bitmask" as uc4
    usecase "UC3: Read/Write Cyclic Registers" as uc3
    usecase "UC2: Read/Write Register blocks" as uc2
    usecase "UC1: Read/Write Registers" as uc1
}
database Registers as reg

user -- uc1
user -- uc2
user -- uc3
user -- uc4

if -- uc5

uc1 -- reg
uc2 -- reg
uc3 -- reg
uc4 -- reg
uc5 -- reg
@enduml
```

### Actors

***User Interface***: User Interface represent any external access to the device. Eg. Serial connection.

***Internal Interface***: Internal interface represent task or threads with special condition. Note that Internal Interface as access to the same use cases as User Interface.

### Database

This part is the registers described for the device.

### Use cases

***UC1: Read/Write Registers***  
This use case allows the different component to read or write single registers via the Register Monitor.

***UC2: Read/Write Register Blocs***
This use case allows the different component to access blocks of registers at once.  
A block is defined a coherent number of registers.

Eg. start at address x and read 3 address, will give register x, x+1 and x+2.

***UC3: Read/Write Cyclic Registers***  
This use cases allows component to collect group of registers that are marked for cyclic handling. 

> Note! That cyclic registers are mainly an industrial practice to read/write registers in a specific cyclic manor! See more about this under chapter x

***UC4: Read/Write Bitmask***  
This Use case allows component to read or write a bitmask to a register.  
Eg. If Status bit is used, reading and writing a register to flip 1 or 2 bits is not quick. Writing a mask into the register can be a lot more effectively.

***UC5: Read/Write Protected Data***
This use case is special, as it allows the system to handle registers that normally would be read-only to be read.  
eg. The Register Monitor is requested from a serial connection to write a register that is read-only. This is stopped. But if a controlling component is requesting to write the same register is not stopped.