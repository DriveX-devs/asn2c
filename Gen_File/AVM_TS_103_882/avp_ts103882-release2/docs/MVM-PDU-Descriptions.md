# <a name="MVM-PDU-Descriptions"></a>ASN.1 module MVM-PDU-Descriptions
OID: _{ itu-t (0) identified-organization (4) etsi (0) itsDomain (5) wg1 (1) ts (103882) mvm (4) major-version-1 (1) minor-version-1(1) }_

## Imports:
* **[AVM-Commons](AVM-Commons.md)** *{ itu-t (0) identified-organization (4) etsi (0) itsDomain (5) wg1 (1) ts (103882) avpCommons (5) major-version-1 (1) minor-version-1(1) }*<br/>
* **[ETSI-ITS-CDD](ETSI-ITS-CDD.md)** *{ itu-t (0) identified-organization (4) etsi (0) itsDomain (5) wg1 (1) 102894 cdd (2) major-version-4 (4) minor-version-1 (1) }*<br/>
## Data Elements:
### <a name="MVM"></a>MVM
This type represents the MVM PDU.

 It shall include the following components:

* _header_ of type [**ItsPduHeader**](ETSI-ITS-CDD.md#ItsPduHeader) <br>
  The header of the MVM PDU.

* _e2eProtection_ of type [**AvmE2EProtection**](AVM-Commons.md#AvmE2EProtection) <br>
  A mandatory container for E2E Protection by Autosar Profile 4.

* _mvm_ of type [**Mvm**](#Mvm) <br>
  The payload of the MVM PDU.

```asn1
MVM ::= SEQUENCE {
  header ItsPduHeader,
  e2eProtection AvmE2EProtection,           
  mvm Mvm
}
```

### <a name="Mvm"></a>Mvm
This type represents the optional container added to the MVM message mainly as a surrogate for missing static vehicle data that RO will possibly need.  

 It includes the following components:

* _mvmDataControlField_ of type [**MVMDataControlField**](#MVMDataControlField)  OPTIONAL<br>
  It contains specific Control Data for [**MVM**](#MVM)   

* _systemManagementData_ of type [**SystemManagementData**](AVM-Commons.md#SystemManagementData)  OPTIONAL<br>
  It contains system management data, i.e. identification labels   				

* _vehicleState_ of type [**VehicleState**](#VehicleState)  OPTIONAL<br>
  Relevant vehicle state information.

* _vidResponse_ of type [**VidResponse**](#VidResponse)  OPTIONAL<br>
  Represents the current vehicle identification status.

* _safetyTimeSyncResponse_ of type [**SafetyTimeSyncResponse**](#SafetyTimeSyncResponse)  OPTIONAL<br>
  Represents a functional safety time related container.

* _safeVehicleTypeConfirmation_ of type [**SafeVehicleTypeConfirmation**](#SafeVehicleTypeConfirmation)  OPTIONAL<br>
  Represents a functional safety related container.

* _vehicleError_ of type [**VehicleError**](#VehicleError)  OPTIONAL<br>
  Error information. Depending on the given error, the infrastructure either tries to resolve the issue or aborts the mission.

* _vehicleSafetyFeedback_ of type [**VehicleSafetyFeedback**](#VehicleSafetyFeedback)  OPTIONAL<br>
  Represents relevant safety information from vehicle.

* _vehicleProperties_ of type [**VehicleProperties**](#VehicleProperties)  OPTIONAL<br>
  Represents vehicle-specific data elements as feedback from vehicle to external system.

* oemSpecific<br>
  Represents an OEM specific 16 bit field for special purposes.
    

```asn1
Mvm ::= SEQUENCE {
  mvmDataControlField MVMDataControlField OPTIONAL,
  systemManagementData SystemManagementData OPTIONAL,		              					   						
  vehicleState VehicleState OPTIONAL,                 						
  vidResponse VidResponse OPTIONAL, 								
  safetyTimeSyncResponse SafetyTimeSyncResponse OPTIONAL,			
  safeVehicleTypeConfirmation SafeVehicleTypeConfirmation OPTIONAL,	
  vehicleError VehicleError OPTIONAL,        						
  vehicleSafetyFeedback VehicleSafetyFeedback OPTIONAL, 			
  vehicleProperties VehicleProperties OPTIONAL,         			 							
  ...                                                           
}
```

### <a name="MVMDataControlField"></a>MVMDataControlField
This type represents a container of specific Control Data for [**MVM**](#MVM).

 It shall include the following components:

* _mvmGenerationTime_ of type [**TimestampIts**](ETSI-ITS-CDD.md#TimestampIts)  OPTIONAL<br>
  the time at which the mvm container was fully assembled.

* _rollingCounterFromMim_ of type **SEQUENCE**  (SIZE(0..10)) OF RollingCounter<br>
  It serves as a mirror of the rolling counter that was 
          received with the latest MIM of the corresponding vehicle. 

* _proprietaryExtensionField_ of type [**ProprietaryExtensionField**](AVM-Commons.md#ProprietaryExtensionField)  OPTIONAL<br>
  This data element defines optionally 2 bytes are used to carry
          specific information or request from a specific OEM vehicle to the RO system .
   

```asn1
MVMDataControlField ::= SEQUENCE {
  mvmGenerationTime  TimestampIts OPTIONAL,
  rollingCounterFromMim SEQUENCE (SIZE(0..10)) OF RollingCounter,             		
  proprietaryExtensionField ProprietaryExtensionField OPTIONAL,             	
  ...
}
```

### <a name="VehicleState"></a>VehicleState
This type represents the container theat carries signals about the actual status of the vehicle's condition with reference to the driving task. This return signals from the vehicle is usable in the RO system for correction and refinement of the vehicle motion control VMC.

 It includes the following components:

* _vehicleStateGenerationTime_ of type [**TimestampIts**](ETSI-ITS-CDD.md#TimestampIts)  OPTIONAL<br>
  Checks on the data freshness on receive side. 

* _operationMode_ of type [**OperationModeEnum**](#OperationModeEnum) <br>
  The current operation mode or state of the vehicle.

* _gearState_ of type [**GearEnum**](AVM-Commons.md#GearEnum) <br>
  Direction in which the vehicle is currently driving or about to drive.

* _directionIndicatorState_ of type [**DirectionIndicatorEnum**](AVM-Commons.md#DirectionIndicatorEnum) <br>
  State of vehicle blinking and direction indication

* _parkingBrakeState_ of type [**ParkingBrakeStateEnum**](#ParkingBrakeStateEnum) <br>
  Status of the vehicle's electric parking brake system.

* _motorSystemState_ of type [**MotorSystemEnum**](AVM-Commons.md#MotorSystemEnum) <br>
  State of vehicle's propulsion motor system

* _currentVelocity_ of type [**VelocityComponentValue**](ETSI-ITS-CDD.md#VelocityComponentValue) <br>
  Current vehicle velocity. Negative when driving backwards.

* _currentCurvature_ of type [**HighResCurvature**](AVM-Commons.md#HighResCurvature) <br>
  Current vehicle curvature.

* _secureStandstill_ of type **BOOLEAN** <br>
  True if the vehicle is currently in secure standstill (i.e. standstill and secured against rolling, even when vehicle is powered down and even on ramps).

* _idxLastWayPoint_ of type [**WaypointIndex**](AVM-Commons.md#WaypointIndex)  OPTIONAL<br>
  Index of last WayPoint that has been received from infrastucture. 0 if not applicable.

* _localizedPose_ of type [**Pose**](AVM-Commons.md#Pose)  OPTIONAL<br>
  The current vehicle pose estimated by the vehicle. (Usually close to the pose estimated by the RO, prediction by the vehicle) 

```asn1
VehicleState ::= SEQUENCE {
  vehicleStateGenerationTime  TimestampIts OPTIONAL, 
  operationMode OperationModeEnum,                    
  gearState GearEnum, 
  directionIndicatorState DirectionIndicatorEnum,  
  parkingBrakeState ParkingBrakeStateEnum,    
  motorSystemState MotorSystemEnum,                       
  currentVelocity VelocityComponentValue,                
  currentCurvature HighResCurvature,                 
  secureStandstill BOOLEAN,                          
  idxLastWayPoint WaypointIndex OPTIONAL,           
  localizedPose Pose OPTIONAL                     
}
```

### <a name="VidResponse"></a>VidResponse
This type represents the optional container supporting a key exchange process that allows the two participants to agree on a secret seed. Note: It is assumed that the communication between the participants is authenticated (payloads are signed). The proposed key exchange is for safety only. It does not contribute to security.

 It includes the following components:

* _vidVehicleState_ of type [**VidVehicleStateEnum**](AVM-Commons.md#VidVehicleStateEnum) <br>
  The current vehicle identification state.

* _vidVehiclePublicKey_ of type [**UInt64**](AVM-Commons.md#UInt64) <br>
  Public Key used by vehicle to derive vehicle identification secret.

```asn1
VidResponse ::= SEQUENCE {
 vidVehicleState VidVehicleStateEnum, 				  
 vidVehiclePublicKey UInt64 						
}
```

### <a name="SafetyTimeSyncResponse"></a>SafetyTimeSyncResponse
This type represents the optional container supporting the calculation of the ITS timestamp given in the expirationTime data element from the DrivingPermission container in the MIM message.

 It includes the following components:

* _challenge_ of type [**UInt16**](AVM-Commons.md#UInt16) <br>
  Challenge received in the SafetyTimeSyncRequest message.

* _vehicleSafetyClockReceiveTimestamp_ of type [**TimestampIts**](ETSI-ITS-CDD.md#TimestampIts) <br>
  Time of the Vehicle Safety Clock when SafetyTimeSyncRequest arrived in the vehicle.

* _vehicleSafetyClockTransmitTimestamp_ of type [**TimestampIts**](ETSI-ITS-CDD.md#TimestampIts) <br>
  Time of the Vehicle Safety Clock when this response left for the RV.

* _checksum_ of type [**UInt32**](AVM-Commons.md#UInt32) <br>
  See safety checksum calculation.

```asn1
SafetyTimeSyncResponse ::= SEQUENCE {
  challenge UInt16,                           			
  vehicleSafetyClockReceiveTimestamp TimestampIts,		
  vehicleSafetyClockTransmitTimestamp TimestampIts,		
  checksum UInt32                             			
}
```

### <a name="SafeVehicleTypeConfirmation"></a>SafeVehicleTypeConfirmation
This type represents the optional container supporting a safe confirmation by the Subject Vehicle.

 It includes the following components:

* _vehicleType_ of type [**IA5String**](#IA5String)  (SIZE(1..32))<br>
  Vehicle type identifier.

* _safetyProfile_ of type [**IA5String**](#IA5String)  (SIZE(1..32))<br>
  VehicleBackend and OperatorBackend negotiate which version of an interface specification 
          or which profile is to be used for the communication between RO and Vehicle. 

* _checksum_ of type [**UInt32**](AVM-Commons.md#UInt32) <br>
  Safety checksum calculation.

```asn1
SafeVehicleTypeConfirmation ::= SEQUENCE {
  vehicleType IA5String (SIZE(1..32)), 	
  safetyProfile IA5String (SIZE(1..32)),
  checksum UInt32                   	
}
```

### <a name="VehicleError"></a>VehicleError
This type represents the optional container data carrier and is added to the MVM message in case an error appears during VMC of the vehicle. 

 It includes the following components:

* _time_ of type [**TimestampIts**](ETSI-ITS-CDD.md#TimestampIts) <br>
  Timestamp when the error occurred.

* _vehCode_ of type [**VehCodeEnum**](#VehCodeEnum) <br>
  Depending on the given error, the infrastructure either tries to resolve the issue or aborts the mission, saves this message for logging and forwards its content to the backend.

* _customCode_ of type [**UInt8**](AVM-Commons.md#UInt8) <br>
  Customer specific error code. The infrastructure won't further interpret this value.

* _description_ of type [**Description**](AVM-Commons.md#Description)  OPTIONAL<br>
  optional description of the error with further details.

```asn1
VehicleError ::= SEQUENCE {
  time TimestampIts,    			
  vehCode VehCodeEnum,  		
  customCode UInt8,     				
  description Description OPTIONAL		
}
```

### <a name="VehicleSafetyFeedback"></a>VehicleSafetyFeedback
Up to 20 elements of [**VehicleSafetyFeedbackContainer**](#VehicleSafetyFeedbackContainer) for down to 5 ms monitoring, logging and debugging.
```asn1
VehicleSafetyFeedback ::= SEQUENCE (SIZE(1..20)) OF VehicleSafetyFeedbackContainer
```

### <a name="VehicleSafetyFeedbackContainer"></a>VehicleSafetyFeedbackContainer
This type represents the optional container added to the MVM message mainly as a surrogate for missing static vehicle data that RO will possibly need.  

 It includes the following components:

* _remainingTimeToStartBraking_ of type [**Millisecond16**](AVM-Commons.md#Millisecond16) <br>
  Represents the time which the vehicle is allowed to keep driving until brakes must be engaged. 

* _safetyViolations_ of type [**SafetyViolationsContainer**](#SafetyViolationsContainer) <br>
  Represents a list of violations which currently lead to stopping the vehicle.

* _currentVehicleSafetyClockTime_ of type [**TimestampIts**](ETSI-ITS-CDD.md#TimestampIts) <br>
  Represents the time when safety component created this container
    

```asn1
VehicleSafetyFeedbackContainer ::= SEQUENCE {
  remainingTimeToStartBraking Millisecond16,                                                           
  safetyViolations SafetyViolationsContainer, 
  currentVehicleSafetyClockTime TimestampIts  
}
```

### <a name="SafetyViolationsContainer"></a>SafetyViolationsContainer
Up to 5 elements of [**SafetyViolationsEnum**](AVM-Commons.md#SafetyViolationsEnum) can be reported per cycle.
```asn1
SafetyViolationsContainer ::= SEQUENCE (SIZE(0..5)) OF SafetyViolationsEnum
```

### <a name="VehicleProperties"></a>VehicleProperties
This type represents the optional container added to the MVM message mainly as a surrogate for missing static vehicle data that RO will possibly need.  

 It includes the following components:

* _basicVehicleClass_ of type [**BasicVehicleClassEnum**](#BasicVehicleClassEnum) <br>
  It shares the vehicle class information to the RO. 

* _vehicleLength_ of type [**Centimetre**](AVM-Commons.md#Centimetre) <br>
  Represents the overall vehicle length in (cm).

* _vehicleWheelbase_ of type [**Centimetre**](AVM-Commons.md#Centimetre) <br>
  Represents the wheelbase value (cm).

* _vehicleRearOverhang_ of type [**Centimetre**](AVM-Commons.md#Centimetre) <br>
  Represents the rear overhang value (cm). 

* _vehicleWidth_ of type [**Centimetre**](AVM-Commons.md#Centimetre) <br>
  Represents the overall vehicle width value (cm).  

* _vehicleTireWidth_ of type [**Centimetre**](AVM-Commons.md#Centimetre) <br>
  Represents a rough vehicle mass value imported fom the CDD.

* _vehicleTrackWidth_ of type [**Centimetre**](AVM-Commons.md#Centimetre) <br>
  Represents the specified width of tires value (cm). 

* _vehicleMass_ of type [**VehicleMass**](ETSI-ITS-CDD.md#VehicleMass) <br>
  Represents a rough vehicle mass value imported fom the CDD.

* _vehicleSpeedLimit_ of type [**VelocityComponentValue**](ETSI-ITS-CDD.md#VelocityComponentValue) <br>
  Represents the limit value of the vehicle speed according to the use case, unit in (cm/s).   

* _vehicleCuvatureLimit_ of type [**HighResCurvature**](AVM-Commons.md#HighResCurvature) <br>
  Represents the maximum curvature for left and right turning (assumed symmetrical).

* _vehicleMaxAngularSteeringRate_ of type [**RadPerSecond**](AVM-Commons.md#RadPerSecond) <br>
  Represents maximum steering rate for automated operation (rate of road wheel angle).
    

```asn1
VehicleProperties ::= SEQUENCE {
  basicVehicleClass BasicVehicleClassEnum,
  vehicleLength  Centimetre,  
  vehicleWheelbase  Centimetre,  
  vehicleRearOverhang  Centimetre,  
  vehicleWidth  Centimetre,  
  vehicleTireWidth  Centimetre, 
  vehicleTrackWidth Centimetre,  
  vehicleMass VehicleMass,
  vehicleSpeedLimit  VelocityComponentValue, 
  vehicleCuvatureLimit  HighResCurvature,   	
  vehicleMaxAngularSteeringRate RadPerSecond,  
  ...
}
```

### <a name="ParkingBrakeStateEnum"></a>ParkingBrakeStateEnum
Data element ParkingBrakeStateEnum describes the status of the electric parking brake .
 It offers the following signal values:
 
 - 0 - `unknown`		    - The status of the electric parking brake is not known.
 - 1 - `engaging`	      - The parking brake is processing to become engaged .
 - 2 - `engaged`		    - The The parking brake is fully engaged .
 - 3 - `disengaging`	  - The parking brake is processing to become disengaged .
 - 4 - `disengaged`	    - The The parking brake is fully disengaged .
```asn1
ParkingBrakeStateEnum ::= ENUMERATED {
  unknown(0),   
  engaging(1),   
  engaged(2), 
  disengaging(3),   	 
  disengaged(4)
}
```

### <a name="OperationModeEnum"></a>OperationModeEnum
The data element OperationModeEnum reports the vehicle' general state. It can also be used for logging purposes and forwarding its content to the backend.

 The value shall be set to:
 - 0 - `unknown(0)`            - The vehicle operation mode is not defined, 
 - 1 - `initializing(1)`  		  - The vehicle is preparing for the mission, but hasn't entered automted mode yet,
 - 2 - `prepared(2)`      		  - The vehicle is in automted mode, but currently doesn't follow one of the control interfaces (waypoints, direct control, ...),
 - 3 - `driving(3)`       		  - The vehicle is in automted mode and actively follows one of the control interfaces (waypoints, direct control, ...). It hasn't reached the end of the given path yet. Also applies if the vehicle stopped temporarily,
 - 4 - `terminating(4)`   		  - The vehicle left automated mode and is terminating related functions,
 - 5 - `suspend (5)` 			    - The vehicle is in a critical error state and requires external operator intervention,
 - 6 - `tempError (6)`         - The vehicle is in a non-critical error state and is initiating a deceleration into stop and hold, prior to suspend,
 - 7 - `humanInControl (7)`    - Manual control of the vehicle has been taken over ,
 - 8 - `stationHold (8)`		    - The external mfg./customer environment interlocks have taken over and is holding the vehicle from marshalling.

Values:
* **unknown** (0)<br>
* **initializing** (1)<br>
* **prepared** (2)<br>
* **driving** (3)<br>
* **terminating** (4)<br>
* **suspend** (5)<br>
* **tempError** (6)<br>
* **humanInControl** (7)<br>
* **stationHold** (8)<br>
```asn1
OperationModeEnum ::= ENUMERATED {
  unknown           (0),
  initializing      (1),  		
  prepared          (2),      	
  driving           (3),       		
  terminating       (4),   		
  suspend           (5), 				
  tempError         (6),           
  humanInControl    (7), 		
  stationHold       (8),		    
  ...
}
```

### <a name="VehCodeEnum"></a>VehCodeEnum
This DE saves this state for logging purposes and forwards its content to the backend.

 The value shall be set to:
 - 0 - `unspecified`        		- Any kind of error that is not specified otherwise. Infrastructure aborts the mission,
 - 1 - `pathNotDriveable`  		- The vehicle can't follow the given waypoints, based on the given DetectedVehiclePose. Infrastructure tries to plan a different path or aborts the mission otherwise,
 - 2 - `onboardVehicleFault` 	- Failure during vehicle onboarding,
 - 3 - `communicationFault`		- vehicle internal comms fault,
 - 4 - `vehicleEgressFault`	  - Failure during vehicle AVP egress or shutdown process.

Values:
* **unspecified** (0)<br>
* **pathNotDriveable** (1)<br>
* **onboardVehicleFault** (2)<br>
* **communicationFault** (3)<br>
* **vehicleEgressFault** (4)<br>
```asn1
VehCodeEnum ::= ENUMERATED {
  unspecified         (0),        		
  pathNotDriveable    (1),  		
  onboardVehicleFault (2), 	
  communicationFault  (3),	
  vehicleEgressFault  (4),		
  ...
}
```

### <a name="BasicVehicleClassEnum"></a>BasicVehicleClassEnum
This data element specified a generic vehicle class that the RO has to cope with.

 The value shall be set to:
 - 0 - `none`                    - Not known or unavailable category,
 - 1 - `unknown`                 - Does not fit any other category,
 - 2 - `special`                 - Special use,
 - 3 - `moto`                    - Motorcycle,
 - 4 - `car`                     - Passenger car,
 - 5 - `carOther`                - Four tire single units,
 - 6 - `bus`                     - Buses,
 - 7 - `axleCnt2`                - Two axle, six tire single units,
 - 8 - `axleCnt3`                - Three axle, single units,
 - 9 - `axleCnt4`                - Four or more axle, single unit,
 - 10 - `axleCnt4Trailer`        - Four or less axle, single trailer,
 - 11 - `axleCnt5Trailer`        - Five or less axle, single trailer,
 - 12 - `axleCnt6Trailer`        - Six or more axle, single trailer,
 - 13 - `axleCnt5MultiTrailer`   - Five or less axle, multi-trailer,
 - 14 - `axleCnt6MultiTrailer`   - Six axle, multi-trailer,
 - 15 - `axleCnt7MultiTrailer`   - Seven or more axle, multi-trailer.

Values:
* **none** (0)<br>
* **unknown** (1)<br>
* **special** (2)<br>
* **moto** (3)<br>
* **car** (4)<br>
* **carOther** (5)<br>
* **bus** (6)<br>
* **axleCnt2** (7)<br>
* **axleCnt3** (8)<br>
* **axleCnt4** (9)<br>
* **axleCnt4Trailer** (10)<br>
* **axleCnt5Trailer** (11)<br>
* **axleCnt6Trailer** (12)<br>
* **axleCnt5MultiTrailer** (13)<br>
* **axleCnt6MultiTrailer** (14)<br>
* **axleCnt7MultiTrailer** (15)<br>
```asn1
BasicVehicleClassEnum ::= ENUMERATED {
  none                  (0), 
  unknown               (1), 
  special               (2), 
  moto                  (3),
  car                   (4),
  carOther              (5), 
  bus                   (6), 
  axleCnt2              (7), 
  axleCnt3              (8), 
  axleCnt4              (9), 
  axleCnt4Trailer       (10), 
  axleCnt5Trailer       (11), 
  axleCnt6Trailer       (12), 
  axleCnt5MultiTrailer  (13), 
  axleCnt6MultiTrailer  (14), 
  axleCnt7MultiTrailer  (15), 
  ...
}
```



