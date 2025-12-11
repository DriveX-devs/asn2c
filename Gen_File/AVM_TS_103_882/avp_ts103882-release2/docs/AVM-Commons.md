# <a name="AVM-Commons"></a>ASN.1 module AVM-Commons
OID: _{ itu-t (0) identified-organization (4) etsi (0) itsDomain (5) wg1 (1) ts (103882) avmCommons (5) major-version-1 (1) minor-version-1(1) }_

## Data Elements:
### <a name="UInt8"></a>UInt8
This DE represents with an unsigned interger value. 
 Size: 8 bit, 1 Byte
```asn1
UInt8 ::= INTEGER (0..255)
```

### <a name="Int16"></a>Int16
This DE represents with a signed interger value. 
 Size: 16 bit, 2 Bytes
```asn1
Int16 ::= INTEGER (-32768..32767)
```

### <a name="UInt16"></a>UInt16
This DE represents with an unsigned interger value. 
 Size: 16 bit, 2 Bytes
```asn1
UInt16 ::= INTEGER (0..65535)
```

### <a name="UInt32"></a>UInt32
This DE represents with an unsigned interger value. 
 Size: 32 bit, 4 Bytes
```asn1
UInt32 ::= INTEGER (0..4294967295)
```

### <a name="UInt64"></a>UInt64
This DE represents with an unsigned interger value. 
 Size: 64 bit, 8 Bytes
```asn1
UInt64 ::= INTEGER(0..18446744073709551615)
```

### <a name="RollingCounter"></a>RollingCounter
This DE represents a rolling counter 
 The value shall be set to 0 when starting an application: 
 This DE is incremented by 1 for each consecutive message. In case of overflow due to the size limit it restarts at 0.
```asn1
RollingCounter ::= UInt16
```

### <a name="WaypointIndex"></a>WaypointIndex
This DE represents a waypoint index 
 The value shall be set according to the sequence of a path snippet or a complete driving trajectory. 
 In case of overflow due to the size limit it restarts at 0.
```asn1
WaypointIndex ::= UInt16
```

### <a name="ProprietaryExtensionField"></a>ProprietaryExtensionField
This DE represents a 2 byte unspecified
 The value shall be set due to specific requirements between a RO sytem and vehicles of a dedicated OEM.
```asn1
ProprietaryExtensionField ::= UInt16
```

### <a name="Pose"></a>Pose
This type represents the container of data elements to indicate the vehicle's overall position in a cartesian coordinate system.
 The location is projected to the ground, elevation is not considered .

 It includes the following components:

* x<br>
  The position in x direction

* y<br>
  The position in x direction

* psi<br>
  The orientation of the vehicle's driving direction.  x (North) axis of the reference coordinate system.

```asn1
Pose ::= SEQUENCE {
```

### <a name="Psi"></a>Psi
This DE represents the vehicle's Counter-clockwise orientation in a cartesian coordinate system 

 The value shall be set to: 
 - `n` (`n ≥ 0` and `n ≤ 62831`) to indicate positive orientation equal to or less than n, and greater than (n-1),
 - `62832` if the  radian is out of range, i.e. greater than 524287 cm,
 - `-62833` when the data is unavailable.  Remark: 2* Pi = 6.28318

Values:
* **xDirection** (0)<br>
* **outOfRange** (62832)<br>
* **unavailable** (62833)<br>

&nbsp;&nbsp;&nbsp;&nbsp;**Unit:** _0.0001 radian_
```asn1
Psi ::= INTEGER {
    xDirection  (0), 
	  outOfRange  (62832),
    unavailable (62833)	
} 
(0..62833)
```

### <a name="Centimetre"></a>Centimetre
This DE represents a distance in cm 

 The value shall be set to: 
 - `n` (`n > -524287` and `n ≤ 0`) to indicate negative distance equal to or less than n x 1 cm, and greater than (n-1) x 1 cm,
 - `n` (`n > 0` and `n < 524287`) to indicate positive distance equal to or less than n x 1 cm, and greater than (n-1) x 1 cm,
 - `-524287` if the  distance is out of negative range
 - `524287` if the  distance is out of positive range 
 - `-524288` when the data is unavailable.

Values:
* **negativeOutOfRange** (-524287)<br>
* **positiveOutOfRange** (524287)<br>
* **unavailable** (-524288)<br>

&nbsp;&nbsp;&nbsp;&nbsp;**Unit:** _1 centimeter_
```asn1
Centimetre ::= INTEGER {
	  negativeOutOfRange (-524287),
    positiveOutOfRange (524287),
    unavailable        (-524288)  
}
(-524288..524287)
```

### <a name="HighResCurvature"></a>HighResCurvature
This DE describes vehicle turning curve with the following information:
 ```
     Value = 1 / Radius * 100000
 ```
 wherein radius is the vehicle turning curve radius in metres. 
 
 Positive values indicate a turning curve to the left hand side of the driver.
 It corresponds to the vehicle coordinate system as defined in ISO 8855 [[21]](#references).

 The value shall be set to:
 - `-32767` for  values smaller than -32767,
 - `n` (`n > -32767` and `n < 0`) for negative values equal to or less than `n`, and greater than `(n-1)`,
 - `0` when the vehicle is moving straight,
 - `n` (`n > 0` and `n < 32767`) for positive values equal to or less than `n`, and greater than `(n-1)`,
 - `32767`, for values  greater than 1021,
 - `-32768`, if the information is not available.

Values:
* **outOfRangeNegative** (-32767)<br>
* **straight** (0)<br>
* **outOfRangePositive** (32767)<br>
* **unavailable** (-32768)<br>

&nbsp;&nbsp;&nbsp;&nbsp;**Unit:** _1 over 100 000 metres_

>>>
NOTE:&emsp;The present DE is limited to vehicle types as defined in ISO 8855 [[21]](#references).
>>>

```asn1
HighResCurvature ::= INTEGER {
    outOfRangeNegative (-32767),
    straight           (0),
    outOfRangePositive (32767), 
    unavailable        (-32768)
} 
(-32768..32767)
```

### <a name="RadPerSecond"></a>RadPerSecond
This DE represents the rotational speed of a steering wheel movement 

 The value shall be set to:
 - `-32 766` to indicate that the rotational speed is equal to or greater than 3.2766 [rad/s] to the right,
 - `n` (`n > -32 766` and `n ≤ 0`) to indicate that the rotation is clockwise (i.e. to the right) and is equal to or less than n x 0,01 rad/s, and greater than (n-1) x 0,01 rad/s,
 - `n` (`n > 0` and `n < 32 766`) to indicate that the rotation is anti-clockwise (i.e. to the left) and is equal to or less than n x 0,01 rad/s, and greater than (n-1) x 0,01 rad/s,
 - `32 766` to indicate that the rotational speed is greater than 3.2765 rad/second to the left,
 - `32 767` to indicate that the information is not available.

Values:
* **negativeOutOfRange** (-32766)<br>
* **positiveOutOfRange** (32766)<br>
* **unavailable** (-32767)<br>

&nbsp;&nbsp;&nbsp;&nbsp;**Unit:** _0.0001 [rad/s]._
```asn1
RadPerSecond ::= INTEGER {
    negativeOutOfRange (-32766), 
    positiveOutOfRange (32766), 
    unavailable        (-32767)
} 
(-32767..32766)
```

### <a name="Millisecond16"></a>Millisecond16
This DE represents a signed time value, size 2 Bytes.

&nbsp;&nbsp;&nbsp;&nbsp;**Unit:** _0,001 s   (1 ms)_
```asn1
Millisecond16 ::= Int16
```

### <a name="GearEnum"></a>GearEnum
Data element GearEnum optionally describes the desired or actual driving direction of the vehicle in alignment with the signed value of the vehicle speed . 
 It offers the following signal values:
 
 - 0 - `park`			  - P: Vehicle engages P ,
 - 1 - `backwards`	- R: Vehicle drives backwards ,
 - 2 - `neutral`		- N: Vehicle engages neutral gear,
 - 3 - `forwards`		- D: Vehicle drives forwards,
 - 4 - `unknown`		- unknown gear status. Not used for commanding a vehicle gear position.

Values:
* **unknown** (4)<br>
```asn1
GearEnum ::= ENUMERATED {
  park(0),       
  backwards(1),  
  neutral(2),    
  forwards(3),   
  unknown (4)	
}
```

### <a name="DirectionIndicatorEnum"></a>DirectionIndicatorEnum
Data element directionIndicatorEnum describes the use of vehicle direction indicator system during VMC .
 It offers the following signal values:
 
 - 0 - `off`		  - The vehicle does not flash indicator lights at this stage. ,
 - 1 - `right`	  - Flash right indicator lights.  ,
 - 2 - `left`		  - Flash left indicator lights.  ,
 - 3 - `both`		  - Flash right and left indicator lights.  ,
 - 4 - `unknown`	- Direction indicator request is unknown. (Do not use for drive command).
```asn1
DirectionIndicatorEnum ::= ENUMERATED {
  off(0),      
  right(1),   
  left(2),    
  both(3),    
  unknown(4)  
}
```

### <a name="MotorSystemEnum"></a>MotorSystemEnum
Data element MotorSystemEnum optionally describes the desired or system stae of the vehicle's propulsion motor (engine). 
 It offers the following signal values:
 
 - 0 - `off`      - Propulsion motor  off 
 - 1 - `on`       - Propulsion motor on
 - 2 - `unknown`  - The propulsion motor state is unknown.
```asn1
MotorSystemEnum ::= ENUMERATED {
  off(0),    
  on(1),   
  unknown(2)	
}
```

### <a name="VidVehicleStateEnum"></a>VidVehicleStateEnum
Data element VidVehicleStateEnum describes the current state in the vehicle identification process .
 It offers the following signal values:
 
 - 0 - `undefined`                    - Default value ,
 - 1 - `ready`                        - Vehicle is ready to get flashing code ,
 - 2 - `lightFlashingInProgress`      - Vehicle flashing is in progress,
 - 3 - `lightFlashingCompleted`       - Flashing is finished,
 - 4 - `lightFlashingFailed`          - This indicates that the vehicle can't flash because of vehicle error,
 - 5 - `authorized`                   - Vehicle identification was successful and vehicle has switched its state.
```asn1
VidVehicleStateEnum ::= ENUMERATED {
  undefined(0),   
  ready(1),        
  lightFlashingInProgress(2), 
  lightFlashingCompleted(3),   
  lightFlashingFailed(4),  
  authorized(5)      
}
```

### <a name="SafetyViolationsEnum"></a>SafetyViolationsEnum
Data element SafetyViolationsEnum reports the outcome of a specific safety cycle.  
 As a result a violation leads to stopping the vehicle.

 It offers the following signal values:
 
 - 0 - `noViolation`                         - Default value ,
 - 1 - `noDrivingPermissionReceived`         - Vehicle has not received a driving permission ,
 - 2 - `lastDrivingPermissionTooOld`         - The driving permission is outdated,
 - 3 - `crcViolationClockSyncRequest`        - The time synchronization request has an invalid checksum,
 - 4 - `crcViolationDrivingPermission`       - The driving permission has an invalid checksum,
 - 5 - `expirationTimeViolation`             - the allowed time is expired.
 - 6 - `drivingDirectionMismatch`            - wrong driving direction is indcated, plausibility error with speed signal,
 - 7 - `velocityViolation`                   - The speed value is out of the allowed range ,
 - 8 - `curvatureMinViolation`               - The allowed minimal curvature is out of range ,
 - 9 - `curvatureMaxViolation`               - The allowed maximal curvature is out of range ,
 - 10 - `expirationTimeTooHigh`              - The experiation time is out of the allowed range,
 - 11 - `monitoring`                         - Violation monitoring is ongoing.
```asn1
SafetyViolationsEnum ::= ENUMERATED {
  noViolation(0),
  noDrivingPermissionReceived(1),
  lastDrivingPermissionTooOld(2),
  crcViolationClockSyncRequest(3),
  crcViolationDrivingPermission(4),
  expirationTimeViolation(5),
  drivingDirectionMismatch(6),
  velocityViolation(7),
  curvatureMinViolation(8),
  curvatureMaxViolation(9),
  expirationTimeTooHigh(10),
  monitoring(11),
  ...
}
```

### <a name="SystemManagementData"></a>SystemManagementData
This type represents a container of system management data, i.e. identification labels.

 It shall include the following components:

* _sessionID_ of type [**SessionMissionID**](#SessionMissionID)  OPTIONAL<br>
  It is a unique identifier that is known by the infrastructure and 
          the vehicle side at the same time. It is valid for a complete parking/marshalling task.

* _missionID_ of type [**SessionMissionID**](#SessionMissionID)  OPTIONAL<br>
  It is a unique identifier that is known by the infrastructure and 
          the vehicle side at the same time. It identifies a subtask within a sessionID.

* _vehicleID_ of type [**VehicleID**](#VehicleID)  OPTIONAL<br>
  It is a unique identifier that is known by the infrastructure and vehicle The vehicle needs to know its own vehicleID.
          This can serve as a safety check for the RO regarding vehicle parameters, as it receives them from the vehicle backend, 
          and it can then verify that it is communicating to the right vehicle through MVM (as the vehicle sends its vehicleID there).      

* _facilityID_ of type [**FacilityID**](#FacilityID)  OPTIONAL<br>
  It is a unique identifier that describes the infrastructure in which the vehicle operates.
          It is used for documentation purposes. The vehicle does not respond to this data element 

```asn1
SystemManagementData ::= SEQUENCE {     
  sessionID SessionMissionID OPTIONAL,
  missionID SessionMissionID OPTIONAL,
  vehicleID VehicleID OPTIONAL,
  facilityID FacilityID OPTIONAL
}
```

### <a name="SessionMissionID"></a>SessionMissionID
This DE represents a the identification of an driving session or mission  
 The value be freely selected as a string of 17 to 32 characters.
```asn1
SessionMissionID ::= IA5String (SIZE(17..32))
```

### <a name="VehicleID"></a>VehicleID
This DE serves for the identification of the specified vehicle
 The value be freely selected as a string of 1 to 17 characters.
```asn1
VehicleID ::= IA5String (SIZE(1..17))
```

### <a name="FacilityID"></a>FacilityID
This DE serves for the identification of infrastructure facility 
 The value be freely selected as a string of 1 to 32 characters.
```asn1
FacilityID ::= IA5String (SIZE(1..32))
```

### <a name="Description"></a>Description
This DE represents a detailed description.
 It can serve to generate a more extensive, user-friendly description e.g. of an error. 
 The value be freely selected as a string of 1 to 200 characters.
```asn1
Description ::=  IA5String (SIZE(1..200))
```

### <a name="AvmE2EProtection"></a>AvmE2EProtection
This type represents the container of data elements supporting E2E protection mechanisms for the Parking Control messages 
 It complies with AUTOSAR Profile 4 e2e protection. It protects the body of the message and starts in a byte aligned position in the BTP stream.

 It includes the following components:

* _length_ of type [**UInt16**](#UInt16) <br>
  represents the overall payload of the body container in the Park Control message. Starting point is this data element.

* _rollingCounter_ of type [**UInt16**](#UInt16) <br>
  This is a recurring identifier of a Park control message. This identifier is incremented by 1 for the consecutive  message. 
                          In case of overflow due to the size limit it restarts at 0.

* _dataID_ of type [**UInt32**](#UInt32) <br>
  The data element is usable to identify the originating system (RO or vehicle). It is to the best effort unique within the network ;
                  It is negotiated between the RO and the vehicle OEM in case they want to identify each other with this signal

* _crc32_ of type [**UInt32**](#UInt32) <br>
  The CRC is calculated in byte limits of the body container, considering the data element length. The signal crc32 itself is excluded from the payload.

```asn1
AvmE2EProtection ::= SEQUENCE {
  length UInt16,
  rollingCounter UInt16,  
  dataID UInt32,
  crc32 UInt32                     
}
```



