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
    usecase "UC6: Read/Write protected data" as uc6
    usecase "UC5: Initialize RegMON" as uc5
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
if -- uc6

uc1 -- reg
uc2 -- reg
uc3 -- reg
uc4 -- reg
uc5 -- reg
uc6 -- reg
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
