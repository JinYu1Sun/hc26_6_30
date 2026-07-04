
"use strict";

let GpsStatus = require('./GpsStatus.js');
let VehicleBrakeCmd = require('./VehicleBrakeCmd.js');
let VehicleCmd = require('./VehicleCmd.js');
let LidarFault = require('./LidarFault.js');
let SystemFault = require('./SystemFault.js');
let localizaionPosition = require('./localizaionPosition.js');
let CameraLane = require('./CameraLane.js');
let PointCloud = require('./PointCloud.js');
let McuInfo = require('./McuInfo.js');
let StationInfo = require('./StationInfo.js');
let MultiMapSelfDetect = require('./MultiMapSelfDetect.js');
let ReferencePoint = require('./ReferencePoint.js');
let MmWave_Objects = require('./MmWave_Objects.js');
let LocationData = require('./LocationData.js');
let Location = require('./Location.js');
let moving_objects = require('./moving_objects.js');
let moving_object = require('./moving_object.js');
let ObstacleList = require('./ObstacleList.js');
let Route2 = require('./Route2.js');
let OBUInfo = require('./OBUInfo.js');
let TaskPoint2 = require('./TaskPoint2.js');
let PK_result = require('./PK_result.js');
let Position = require('./Position.js');
let mmwave_object = require('./mmwave_object.js');
let DiagnosticResult = require('./DiagnosticResult.js');
let VehicleReport = require('./VehicleReport.js');
let PolygonWithHoles = require('./PolygonWithHoles.js');
let NotifyPlan = require('./NotifyPlan.js');
let TaskPoint3 = require('./TaskPoint3.js');
let BoundingBox = require('./BoundingBox.js');
let Camera = require('./Camera.js');
let PlanningResult = require('./PlanningResult.js');
let VehicleCommand = require('./VehicleCommand.js');
let Route = require('./Route.js');
let VisualPreception = require('./VisualPreception.js');
let Ultrasonic = require('./Ultrasonic.js');
let GPSpublish = require('./GPSpublish.js');
let VehicleAutoEn = require('./VehicleAutoEn.js');
let MovPlan = require('./MovPlan.js');
let GlonavinGpsDebug = require('./GlonavinGpsDebug.js');
let PolygonWithHolesStamped = require('./PolygonWithHolesStamped.js');
let CameraFault = require('./CameraFault.js');
let VehicleControl = require('./VehicleControl.js');
let LidarSelfDetect = require('./LidarSelfDetect.js');
let TaskPoint = require('./TaskPoint.js');
let VehicleInfo = require('./VehicleInfo.js');
let UwbPosition = require('./UwbPosition.js');
let PlanningMapPointList = require('./PlanningMapPointList.js');
let LocalPath = require('./LocalPath.js');
let MapPath = require('./MapPath.js');
let LocalPose = require('./LocalPose.js');
let VehicleInfoTest = require('./VehicleInfoTest.js');
let BoundingBoxes = require('./BoundingBoxes.js');
let StrategicJudgment = require('./StrategicJudgment.js');
let Route3 = require('./Route3.js');
let Obstacle = require('./Obstacle.js');
let GpsPosition = require('./GpsPosition.js');
let PlanningMapPoint = require('./PlanningMapPoint.js');
let RD_result = require('./RD_result.js');
let ndtTimeAndScore = require('./ndtTimeAndScore.js');
let CommonFault = require('./CommonFault.js');
let Vslam = require('./Vslam.js');
let ReferenceLineDualLocalization = require('./ReferenceLineDualLocalization.js');
let LocalPlan = require('./LocalPlan.js');
let PerceptionStatus = require('./PerceptionStatus.js');
let PerceptionSelfDetect = require('./PerceptionSelfDetect.js');
let ReferenceLine = require('./ReferenceLine.js');
let ReferencePointDualLocalization = require('./ReferencePointDualLocalization.js');

module.exports = {
  GpsStatus: GpsStatus,
  VehicleBrakeCmd: VehicleBrakeCmd,
  VehicleCmd: VehicleCmd,
  LidarFault: LidarFault,
  SystemFault: SystemFault,
  localizaionPosition: localizaionPosition,
  CameraLane: CameraLane,
  PointCloud: PointCloud,
  McuInfo: McuInfo,
  StationInfo: StationInfo,
  MultiMapSelfDetect: MultiMapSelfDetect,
  ReferencePoint: ReferencePoint,
  MmWave_Objects: MmWave_Objects,
  LocationData: LocationData,
  Location: Location,
  moving_objects: moving_objects,
  moving_object: moving_object,
  ObstacleList: ObstacleList,
  Route2: Route2,
  OBUInfo: OBUInfo,
  TaskPoint2: TaskPoint2,
  PK_result: PK_result,
  Position: Position,
  mmwave_object: mmwave_object,
  DiagnosticResult: DiagnosticResult,
  VehicleReport: VehicleReport,
  PolygonWithHoles: PolygonWithHoles,
  NotifyPlan: NotifyPlan,
  TaskPoint3: TaskPoint3,
  BoundingBox: BoundingBox,
  Camera: Camera,
  PlanningResult: PlanningResult,
  VehicleCommand: VehicleCommand,
  Route: Route,
  VisualPreception: VisualPreception,
  Ultrasonic: Ultrasonic,
  GPSpublish: GPSpublish,
  VehicleAutoEn: VehicleAutoEn,
  MovPlan: MovPlan,
  GlonavinGpsDebug: GlonavinGpsDebug,
  PolygonWithHolesStamped: PolygonWithHolesStamped,
  CameraFault: CameraFault,
  VehicleControl: VehicleControl,
  LidarSelfDetect: LidarSelfDetect,
  TaskPoint: TaskPoint,
  VehicleInfo: VehicleInfo,
  UwbPosition: UwbPosition,
  PlanningMapPointList: PlanningMapPointList,
  LocalPath: LocalPath,
  MapPath: MapPath,
  LocalPose: LocalPose,
  VehicleInfoTest: VehicleInfoTest,
  BoundingBoxes: BoundingBoxes,
  StrategicJudgment: StrategicJudgment,
  Route3: Route3,
  Obstacle: Obstacle,
  GpsPosition: GpsPosition,
  PlanningMapPoint: PlanningMapPoint,
  RD_result: RD_result,
  ndtTimeAndScore: ndtTimeAndScore,
  CommonFault: CommonFault,
  Vslam: Vslam,
  ReferenceLineDualLocalization: ReferenceLineDualLocalization,
  LocalPlan: LocalPlan,
  PerceptionStatus: PerceptionStatus,
  PerceptionSelfDetect: PerceptionSelfDetect,
  ReferenceLine: ReferenceLine,
  ReferencePointDualLocalization: ReferencePointDualLocalization,
};
