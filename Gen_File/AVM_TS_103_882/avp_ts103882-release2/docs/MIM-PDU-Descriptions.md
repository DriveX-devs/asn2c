# <a name="MIM-PDU-Descriptions"></a>ASN.1 module MIM-PDU-Descriptions
OID: _{ itu-t (0) identified-organization (4) etsi (0) itsDomain (5) wg1 (1) ts (103882) mim (6) major-version-1 (1) minor-version-1 (1) }_

## Imports:
* **[AVM-Commons](AVM-Commons.md)** *{ itu-t (0) identified-organization (4) etsi (0) itsDomain (5) wg1 (1) ts (103882) avpCommons (5) major-version-1 (1) minor-version-1(1) }*<br/>
* **[ETSI-ITS-CDD](ETSI-ITS-CDD.md)** *{ itu-t (0) identified-organization (4) etsi (0) itsDomain (5) wg1 (1) 102894 cdd (2) major-version-4 (4) minor-version-1 (1) }*<br/>
## Data Elements:
### <a name="MIM"></a>MIM
This type represents the MIM PDU.

 It shall include the following components:

* _header_ of type [**ItsPduHeader**](ETSI-ITS-CDD.md#ItsPduHeader) <br>
  the header of the MIM PDU.

* _e2eProtection_ of type [**AvmE2EProtection**](AVM-Commons.md#AvmE2EProtection) <br>
  A mandatory container for E2E Protection by Autosar Profile 4

* _mims_ of type **SEQUENCE**  (SIZE(1..32)) OF Mim<br>
  a sequence of Mim containers which addresses min 1 and max. 32 vehicles .
   

```asn1
MIM ::= SEQUENCE {
  header ItsPduHeader,
  e2eProtection AvmE2EProtection,           
  mims SEQUENCE (SIZE(1..32)) OF Mim  
}
```

### <a name="Mim"></a>Mim
This type represents the Mim Container.

 It shall include the following components:

* _mimDataControlField_ of type [**MIMDataControlField**](#MIMDataControlField)  OPTIONAL<br>
  a container of specific Control Data for MIM.

* _systemManagementData_ of type [**SystemManagementData**](AVM-Commons.md#SystemManagementData)  OPTIONAL<br>
  a container of identification data

* _vehicleIdentification_ of type [**VidRequest**](#VidRequest)  OPTIONAL<br>
  It allows the selection of the mechanism for visual vehicle identification. 

* _drivingPermission_ of type [**DrivingPermission**](#DrivingPermission)  OPTIONAL<br>
  The driving permission container offers a safety concept to allow or deny vehicle motion.
          It defines the bounds within which the vehicle is allowed to drive.

* _safetyTimeSyncRequest_ of type [**SafetyTimeSyncRequest**](#SafetyTimeSyncRequest)  OPTIONAL<br>
  The Remote Vehicle Operation needs to be able to continuously
          determine the current time in which the Subject Vehicle operates.
          This container supports functional safety concepts.

* _driveCommand_ of type [**DriveCommand**](#DriveCommand)  OPTIONAL<br>
  It addresses the specific vehicle and determines the actual state
          request to the vehicle, especially the active drive request for the overall VCM operation.

* _detectedVehiclePose_ of type [**DetectedVehiclePose**](#DetectedVehiclePose)  OPTIONAL<br>
  The RO permanently senses the vehicle's position
          in a proprietary two-dimensional coordinate system (X and Y). 

* _controlInterface_ of type [**ControlInterface**](#ControlInterface)  OPTIONAL<br>
  This container offers different control methods for vehicle motion control VMC as a choice between .

```asn1
Mim ::= SEQUENCE {
  mimDataControlField MIMDataControlField OPTIONAL,
  systemManagementData SystemManagementData OPTIONAL,					          	
  vehicleIdentification VidRequest OPTIONAL,         	
  drivingPermission DrivingPermission OPTIONAL,      	
  safetyTimeSyncRequest SafetyTimeSyncRequest OPTIONAL, 
  driveCommand DriveCommand OPTIONAL,                         	
  detectedVehiclePose DetectedVehiclePose OPTIONAL,  	
  controlInterface ControlInterface OPTIONAL,                 	             	
  ...
}
```

### <a name="MIMDataControlField"></a>MIMDataControlField
This type represents a container of specific Control Data for MIM.

 It shall include the following components:

* _checksum_ of type [**UInt32**](AVM-Commons.md#UInt32)  OPTIONAL<br>
  the optional checksum of a single mim vehicle container.

* _mimGenerationTime_ of type [**TimestampIts**](ETSI-ITS-CDD.md#TimestampIts)  OPTIONAL<br>
  the time at which the vehicle container was fully assembled.

* _rollingCounterFromMvm_ of type **SEQUENCE**  (SIZE(0..10)) OF RollingCounter<br>
  It serves as a mirror of the rolling counter that was 
          received with the latest MVM of the corresponding vehicle. 

* _proprietaryExtensionField_ of type [**ProprietaryExtensionField**](AVM-Commons.md#ProprietaryExtensionField)  OPTIONAL<br>
  This data element defines optionally 2 bytes are used to carry
          specific information or request from the RO system to a specific OEM vehicle.
   

```asn1
MIMDataControlField ::= SEQUENCE {
  checksum UInt32 OPTIONAL,
  mimGenerationTime  TimestampIts OPTIONAL,
  rollingCounterFromMvm SEQUENCE (SIZE(0..10)) OF RollingCounter,
  proprietaryExtensionField ProprietaryExtensionField OPTIONAL,
  ...
}
```

### <a name="VidRequest"></a>VidRequest
This type represents a selection of methods for vehicle identification during standstill.

 It shall include the following components:

* _blinking_ of type [**Blinking**](#Blinking) <br>
  At current stage only the "blinking lights" method is selectable for vehicle identification

```asn1
VidRequest ::= CHOICE {     
  blinking Blinking,
  ...
}
```

### <a name="Blinking"></a>Blinking
This type represents the container with data elements for the blinking vehicle identification method.

 It shall include the following components:

* _vidRoPublicKey_ of type [**UInt64**](AVM-Commons.md#UInt64) <br>
  A 64-bit public key is transmitted from the RO to the vehicle
          in order to derive the vehicle identification secret.

* _codeLength_ of type [**UInt8**](AVM-Commons.md#UInt8) <br>
  The codelength data element sends a value that indicates how many bits (8 - 20)
          from the seed shall be used for generating the blinking pattern.

* _blinkingCommand_ of type [**VidRequestCommandEnum**](#VidRequestCommandEnum) <br>
* currentState<br>
  This data element mirrors the current authorization/identification
          request status from the perspective of the RO system.

```asn1
Blinking ::= SEQUENCE {
  vidRoPublicKey UInt64,					 
  codeLength UInt8,                          
  blinkingCommand VidRequestCommandEnum           
}
```

### <a name="DrivingPermission"></a>DrivingPermission
This type represents the container with data elements for the vehicle obtaining or denying a permission to drive.

 The driving permission concept offers an expiration time for driving permission and the boundary values of longitudinal and lateral movement of the subject vehicle. 

 It shall include the following components:

* _expirationTime_ of type [**TimestampIts**](ETSI-ITS-CDD.md#TimestampIts) <br>
  At this time the vehicle shall start braking, unless a new and updated expirationTime time stamp was sent.

* _velocityMax_ of type [**VelocityComponentValue**](ETSI-ITS-CDD.md#VelocityComponentValue) <br>
  During the allowed expiration time the vehicle shall not exceed the velocityMax speed value in forward as well in reverse driving conditions.

* _curvatureMin_ of type [**HighResCurvature**](AVM-Commons.md#HighResCurvature) <br>
  During the allowed expiration time the vehicle steering system shall not exceed the limits of interval given by data element curvatureMin.

* _curvatureMax_ of type [**HighResCurvature**](AVM-Commons.md#HighResCurvature) <br>
  During the allowed expiration time the vehicle steering system shall not exceed the limits of interval given by data element curvatureMax.

* _checksum_ of type [**UInt32**](AVM-Commons.md#UInt32) <br>
  The driving permission needs to be protected by a dedicated safety checksum.

```asn1
DrivingPermission ::= SEQUENCE {
  expirationTime TimestampIts,         
  velocityMax VelocityComponentValue,  
  curvatureMin HighResCurvature,       
  curvatureMax HighResCurvature,       
  checksum UInt32                      
}
```

### <a name="SafetyTimeSyncRequest"></a>SafetyTimeSyncRequest
This type represents the container with data elements for requesting a safety time synchronization.

 It shall include the following components:

* _challenge_ of type [**UInt16**](AVM-Commons.md#UInt16) <br>
  The Remote Vehicle Operation shall use this challenge to relate the response to this request. This challenge is chosen by the RO in accordance with the safety requirements. 

* _checksum_ of type [**UInt32**](AVM-Commons.md#UInt32) <br>
  This safety time synchronization needs to be protected by a dedicated safety checksum.

```asn1
SafetyTimeSyncRequest ::= SEQUENCE {
  challenge UInt16, 
  checksum UInt32   
}
```

### <a name="DriveCommand"></a>DriveCommand
This type represents the container with data elements for general drive commands.

 It addresses the specific vehicle and determines the actual state request to the vehicle, especially the active drive request for the VCM operation. 

 It shall include the following components:

* _driveCommandAction_ of type [**DriveCommandActionEnum**](#DriveCommandActionEnum) <br>
  The main drive state request, inf the loop of initialize and terminate.

* _terminateReason_ of type [**TerminateReasonEnum**](#TerminateReasonEnum) <br>
  If action equals terminate, indicates whether a terminate is requested because the vehicle reached the destination or because of an error.

* _gearRequest_ of type [**GearEnum**](AVM-Commons.md#GearEnum)  OPTIONAL<br>
  It describes the desired driving direction of the vehicle in alignment with the signed value of the vehicle speed requests.

* _directionIndicatorRequest_ of type [**DirectionIndicatorEnum**](AVM-Commons.md#DirectionIndicatorEnum)  OPTIONAL<br>
  It signalizes the currently requested direction indicator.

* _parkingBrakeRequest_ of type [**ParkingBrakeRequestEnum**](#ParkingBrakeRequestEnum)  OPTIONAL<br>
  It signalizes the currently requested actuation of the vehicle's electric parking brake.

* _motorSystemRequest_ of type [**MotorSystemEnum**](AVM-Commons.md#MotorSystemEnum)  OPTIONAL<br>
  It signalizes the currently requested actuation of the vehicle's propulsion motor.

* _emergencyStopRequest_ of type [**EmergencyStopEnum**](#EmergencyStopEnum)  OPTIONAL<br>
  This  enables as part of an optional safety concept to apply a vehicle specific emergency stop manoeuvre at any time.

* _interlockRequest_ of type [**InterlockEnum**](#InterlockEnum)  OPTIONAL<br>
  This data element enables an optional safety concept to apply a vehicle interlock.

* _hornRequest_ of type [**VehicleHornRequestEnum**](#VehicleHornRequestEnum)  OPTIONAL<br>
  This data element enables an optional safety concept. It commands the vehicle to sound the horn in different formats in order to warn pedestrians or animals.

```asn1
DriveCommand ::= SEQUENCE {
  driveCommandAction DriveCommandActionEnum,   		
  terminateReason TerminateReasonEnum,         		 
  gearRequest GearEnum OPTIONAL,                     
  directionIndicatorRequest DirectionIndicatorEnum OPTIONAL,  
  parkingBrakeRequest ParkingBrakeRequestEnum OPTIONAL,
  motorSystemRequest MotorSystemEnum OPTIONAL,
  emergencyStopRequest EmergencyStopEnum OPTIONAL,	
  interlockRequest InterlockEnum OPTIONAL,          
  hornRequest VehicleHornRequestEnum OPTIONAL   	
}
```

### <a name="DetectedVehiclePose"></a>DetectedVehiclePose
This type represents the container with a report of the vehicle's localization.

 The RO permanently senses the vehicle's position in a relative two-dimensional coordinate system (X and Y). 

 It shall include the following components:

* _detectedPose_ of type [**Pose**](AVM-Commons.md#Pose) <br>
  This field implements the sequence of data elements for the vehicle' positioning. The field indicates an agreed reference point of the vehicle,
   e.g. the middle of the rear axle.   

* _poseMeasurementTime_ of type [**TimestampIts**](ETSI-ITS-CDD.md#TimestampIts) <br>
  The data element signals an absolute and synchronized time stamp when the measurement was taken.

```asn1
DetectedVehiclePose ::= SEQUENCE {
  detectedPose Pose,
  poseMeasurementTime TimestampIts
}
```

### <a name="ControlInterface"></a>ControlInterface
This type selects the container that is used for vehicle motion control.

 With an extension marker additional choices for further control methods are applicable in later versions of the protocol. 

 It shall include the following components:

* _pathControl_ of type [**PathControl**](#PathControl) <br>
  The pathSnippet control method transfers a sequence of wayPoint containers from the infrastructure into the vehicle.   

* _trajectoryControl_ of type [**TrajectoryControl**](#TrajectoryControl) <br>
  A vehicle trajectory consists of a vector of ControlTrajectory elements and StateTrajectory elements. It also contains a reference time stamp and an optional drive direction element.

```asn1
ControlInterface ::= CHOICE {
  pathControl PathControl,
  trajectoryControl TrajectoryControl,
  ...
}
```

### <a name="PathControl"></a>PathControl
The type PathControl sequeence represents the container with Path Snippet elements.

 It shall include the following components:

* _pathSnippet_ of type [**PathSnippet**](#PathSnippet)  OPTIONAL<br>
  a sequence of up to 200 way points

* _clearedDistanceOnPath_ of type [**Centimetre**](AVM-Commons.md#Centimetre) <br>
  the RO can tell the vehicle to stop on the known PathSnippet without sending a new PathSnippet.

* _situationalVelocityLimit_ of type [**VelocityComponentValue**](ETSI-ITS-CDD.md#VelocityComponentValue)  OPTIONAL<br>
  the RO can tell the vehicle to temporarily drive slower without sending a new PathSnippet. 

```asn1
PathControl ::= SEQUENCE { 
  pathSnippet PathSnippet OPTIONAL, 
  clearedDistanceOnPath Centimetre, 
  situationalVelocityLimit VelocityComponentValue OPTIONAL 
}
```

### <a name="PathSnippet"></a>PathSnippet
The type PathSnippet represents the container with Path Snippet elements.

 It shall contain up to 200 way points of type [**WayPoint**](#WayPoint)
```asn1
PathSnippet ::= SEQUENCE (SIZE(0..200)) OF WayPoint
```

### <a name="WayPoint"></a>WayPoint
This type represents the container with elements describing a way point, 

 It shall include the following components:

* _index_ of type [**WaypointIndex**](AVM-Commons.md#WaypointIndex)  OPTIONAL<br>
  It is used to build an array of consecutive way points.

* _wayPointPose_ of type [**Pose**](AVM-Commons.md#Pose) <br>
  This field implements the positioning request to the vehicle. The waypoint indicates an agreed reference point of the vehicle,
   e.g. the middle of the rear axle.  

* _velocity_ of type [**VelocityComponentValue**](ETSI-ITS-CDD.md#VelocityComponentValue) <br>
  This data element represents the target speed of the vehicle in that specific way point.
   It is a signed velocity value compliant with the drive direction request. Positive: drive forwards / Negative: reverse.

* _curvature_ of type [**HighResCurvature**](AVM-Commons.md#HighResCurvature) <br>
  This data element represents the target curvature of the vehicle in that specific way point.

* _pitchAngle_ of type [**CartesianAngleValue**](ETSI-ITS-CDD.md#CartesianAngleValue)  OPTIONAL<br>
  For better velocity control, it should be possible to specify the inclination (or vehicle pitch angle) at specific locations.

```asn1
WayPoint ::= SEQUENCE {
  index WaypointIndex OPTIONAL,                 
  wayPointPose Pose,
  velocity VelocityComponentValue,     
  curvature HighResCurvature,
  pitchAngle CartesianAngleValue OPTIONAL          
}
```

### <a name="TrajectoryControl"></a>TrajectoryControl
The type TrajectoryControl represents the container with elements for the trajectory control method. 

 It shall include the following components:

* _timeReference_ of type [**TimestampIts**](ETSI-ITS-CDD.md#TimestampIts) <br>
  The reference time indicates the absolute time given in Vehicle Functional Clock 
   at which the first element of the [**ControlTrajectory**](#ControlTrajectory) vector and [**StateTrajectory**](#StateTrajectory) vector is expected to be executed.

* _driveDirection_ of type [**DriveDirectionEnum**](#DriveDirectionEnum)  OPTIONAL<br>
  The data element describes the desired driving direction of the vehicle in alignment with the signed value of the vehicle acceleration requests. 

* _controlTrajectory_ of type [**ControlTrajectory**](#ControlTrajectory) <br>
  This consists of a sequence of [**ControlPoint**](#ControlPoint) elements with a maximum size of 50. 

* _stateTrajectory_ of type [**StateTrajectory**](#StateTrajectory)  OPTIONAL<br>
  The elements in the optional field [**StateTrajectory**](#StateTrajectory) vector are considered as odometry target values.
          These consist of a sequence of [**StatePoint**](#StatePoint) elements with a maximum size of 50.

```asn1
TrajectoryControl ::= SEQUENCE {
  timeReference TimestampIts,
  driveDirection DriveDirectionEnum OPTIONAL,
  controlTrajectory ControlTrajectory,
  stateTrajectory StateTrajectory OPTIONAL
}
```

### <a name="ControlTrajectory"></a>ControlTrajectory
```asn1
ControlTrajectory ::= SEQUENCE (SIZE(0..50)) OF ControlPoint
```

### <a name="ControlPoint"></a>ControlPoint
A ControlPoint container carries a set of ControlPoint elements, 

 It shall include the following components:

* _curvature_ of type [**HighResCurvature**](AVM-Commons.md#HighResCurvature) <br>
  The data element contains the target curvature that is requested in the according control Poiint

* _controlParameter_ of type [**ControlParameter**](#ControlParameter) <br>
  This field selects the method how to represent a [**ControlPoint**](#ControlPoint) container .

```asn1
ControlPoint ::= SEQUENCE {
  curvature HighResCurvature,            
  controlParameter ControlParameter					  
}
```

### <a name="ControlParameter"></a>ControlParameter
The ControlParameter container offers two  options to represent the second parameter within a [**ControlPoint**](#ControlPoint), 

 It shall include the following components:

* _controlAcceleration_ of type [**ControlAcceleration**](#ControlAcceleration) <br>
  The data element represents the vehicles's target acceleration in the according control point.

* _controlVelocity_ of type [**ControlVelocity**](#ControlVelocity) <br>
  Theis field is selected when the according [**ControlPoint**](#ControlPoint) uses speed values instead of acceleration.

```asn1
ControlParameter ::= CHOICE {
  controlAcceleration ControlAcceleration,
  controlVelocity ControlVelocity
}
```

### <a name="ControlAcceleration"></a>ControlAcceleration
The [**ControlAcceleration**](#ControlAcceleration) data element is a single data element when choosing the [**ControlParameter**](#ControlParameter) controlAcceleration
 It deals with a signed acceleration value.
```asn1
ControlAcceleration ::= LongitudinalAccelerationValue
```

### <a name="ControlVelocity"></a>ControlVelocity
The ControlVelocity container consists of 2 data elements belonging to the according [**ControlPoint**](#ControlPoint), 

 It includes the following components:

* _velocity_ of type [**VelocityComponentValue**](ETSI-ITS-CDD.md#VelocityComponentValue) <br>
  The data element represents the target speed of the vehicle. It is a signed velocity value. 
   It deals with a signed velocity value.
   - Positive: drive forwards
   - Negative: driving reverse.

* _distanceToStop_ of type [**Centimetre**](AVM-Commons.md#Centimetre)  OPTIONAL<br>
  The data element represents the unsigned maximum distance that the vehicle can drive before a standstill. 

```asn1
ControlVelocity ::= SEQUENCE {
  velocity VelocityComponentValue,  
  distanceToStop Centimetre OPTIONAL	
}
```

### <a name="StateTrajectory"></a>StateTrajectory
a vector, considered as odometry target values. It consist of a sequence of statePoint containers with a maximum size of 50.
```asn1
StateTrajectory ::= SEQUENCE (SIZE(0..50)) OF StatePoint
```

### <a name="StatePoint"></a>StatePoint
A StatePoint container carries a set of SatePoint elements, 

 It shall include the following components:

* _statePose_ of type [**Pose**](AVM-Commons.md#Pose) <br>
  This field implements the sequence of data elements for the vehicle's positioning in the according statePoint. 
   A point indicates an agreed reference point of the vehicle,e.g. the middle of the rear axle.

* _velocity_ of type [**VelocityComponentValue**](ETSI-ITS-CDD.md#VelocityComponentValue) <br>
  The data element represents the target speed of the vehicle in the according statePoint. It is a signed velocity value.

```asn1
StatePoint ::= SEQUENCE {
  statePose Pose,
  velocity VelocityComponentValue  
}
```

### <a name="DriveCommandActionEnum"></a>DriveCommandActionEnum
Data element DriveCommandActionEnum describes the current state in the vehicle identification process .
 It offers the following signal values:
 
 - 0 - `sleep`		- The vehicle is commanded to go into sleep mode for low power consumption
 - 1 - `initialize`	- The vehicle is awake and remains in standstill. It shall initialize and prepare for a driving job, including a sequence of power-on the engine system and the lights,
 - 2 - `wait`			- The vehicle shall wait in marshalling capable state,
 - 3 - `drive`		- The vehicle is commanded to actively drive and follow the according control commands. Standstill and pause situations are potentially included in this stage,
 - 4 - `terminate`	- The vehicle shall disable the according control interface. After brining the vehicle to a safe standstill state it shuts down as soon as possible.
```asn1
DriveCommandActionEnum ::= ENUMERATED {
  sleep(0),     
  initialize(1),  
  wait(2), 		  
  drive(3),       
  terminate(4)    
}
```

### <a name="TerminateReasonEnum"></a>TerminateReasonEnum
Data element TerminateReasonEnum describes verbally the rational why to terminate the VMC process  .
 It offers the following signal values:
 
 - 0 - `proceed`				- Everything is okay. Proceed, do not terminate,
 - 1 - `destinationReached`	- Vehicle has reached its destinations,
 - 2 - `infrastructureError`	- Error in infrastructure detected,
 - 3 - `vehicleError`		- Vehicle has sent an error code,
 - 4 - `backend`				- Error in backend,
 - 5 - `vehicleIdentificationError`   - Either a wrong or no code was detected. Even with a crash of the camera, there is an error.
```asn1
TerminateReasonEnum ::= ENUMERATED {
  proceed(0),               
  destinationReached(1),   
  infrastructureError(2),   
  vehicleError(3),         
  backend(4),
  vehicleIdentificationError(5)                
}
```

### <a name="ParkingBrakeRequestEnum"></a>ParkingBrakeRequestEnum
Data element ParkingBrakeRequestEnum describes the status of the EmergencyStop Request during VMC .
 It offers the following signal values:
 
 - 0 - `disengage`		- The vehicle shall disengage the electric parking brake.
 - 1 - `engage`			  - The vehicle shall activate the electric parking brake.
```asn1
ParkingBrakeRequestEnum ::= ENUMERATED {
  disengage(0),   	 
  engage(1)
}
```

### <a name="EmergencyStopEnum"></a>EmergencyStopEnum
Data element EmergencyStopEnum describes the status of the EmergencyStop Request during VMC .
 It offers the following signal values:
 
 - 0 - `inactive`		- The vehicle shall not use EmergencyStop at this stage. ,
 - 1 - `precharge`	- The vehicle brake  system shall stay in standby (brake prefill) and wait for an active command.  ,
 - 2 - `active`		  - The vehicle shall initiate an emergency stop using the maximum possible deceleration. ,
 - 3 - `tempError`	- Maturing Failures - Vehicle shall decellerate and hold, it is waiting for the failure to clear or mature. ,
 - 4 - `suspend`		- Command for critical failures (not recoverable) in RO system â The vehicle shall decelerate and secure.
```asn1
EmergencyStopEnum ::= ENUMERATED {
  inactive(0),   	 
  precharge(1),  	
  active(2),      	
  tempError(3),	
  suspend(4)		
}
```

### <a name="InterlockEnum"></a>InterlockEnum
Data element InterlockEnum describes the status of vehicle interlock system during VMC .
 It offers the following signal values:
 
 - 0 - `none`				    - The vehicle shall not use Interlock at this stage,
 - 1 - `zonalInterlock`	- Zonal Interlock is specific to the factory use case,
 - 2 - `globalStop`		  - Command for global stop - external operator intervention is required.
```asn1
InterlockEnum ::= ENUMERATED {
  none(0),   			 
  zonalInterlock(1),  	
  globalStop(2)      	  
}
```

### <a name="VehicleHornRequestEnum"></a>VehicleHornRequestEnum
Data element VehicleHornRequestEnum describes the use of vehicle alarm horn during VMC .
 It offers the following signal values:
 
 - 0 - `none`				  - The vehicle does not sound a horn at this stage. ,
 - 1 - `singleHorn`		- The vehicle applies a single horn for alerting surroundings when marshalling of vehicle is starting.  ,
 - 2 - `doubleHorn`		- For notifying pedestrians who pose obstacles to the vehicle marshalling task.  ,
 - 3 - `holdHorn`		  - In event of ground staff intervention is needed for the vehicle.
```asn1
VehicleHornRequestEnum ::= ENUMERATED {
  none(0),   			
  singleHorn(1),	
  doubleHorn(2),	
  holdHorn(3)		
}
```

### <a name="VidRequestCommandEnum"></a>VidRequestCommandEnum
Data element VidRequestCommandEnum describes the vehicle identification request.
 It offers the following signal values:
 
 - 0 - `generateNewCode`                       - A new safe vehicle identification cycle was started. There is no intent for flashing in this cycle.
 - 1 - `generateNewCodeAndPrepareForFlashing`  - A new safe vehicle identification cycle was started. Flashing will be required in this cycle.
 - 2 - `flashing`	                             - The infrastructure is prepared and waiting for the Subject Vehicle to flash the code.  ,
 - 3 - `successful`                            - The Subject Vehicle was recognized correctly and the identification is completed.  ,
```asn1
VidRequestCommandEnum ::= ENUMERATED {
  generateNewCode(0),     
  generateNewCodeAndPrepareForFlashing(1), 
  flashing(2), 		
  successful(3)
}
```

### <a name="DriveDirectionEnum"></a>DriveDirectionEnum
Data element DriveDirectionEnum describes the request of the vehicle drive direction. 
 This must be in line with the signed vehicle speed request .
 It offers the following signal values:
 
 - 0 - `forwards`			- D: Vehicle shall drive forwards. ,
 - 1 - `backwards`		- R: Vehicle shall drive backwards.
```asn1
DriveDirectionEnum ::= ENUMERATED {
  forwards(0),          
  backwards(1)		    
}
```



