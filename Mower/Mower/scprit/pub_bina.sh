rostopic pub -1 /planning_hull_info util/PolygonWithHolesStamped "header:
  seq: 0
  stamp:
    secs: 0
    nsecs: 0
  frame_id: 'map'   
polygon:
  hull:
    points:
    - x: 0.0
      y: 0.0
      z: 0.0
    - x: 20.0
      y: 0.0
      z: 0.0
    - x: 20.0
      y: 15.0
      z: 0.0
    - x: 0.0
      y: 15.0
      z: 0.0
  holes:
  - points:
    - x: 20.0
      y: 0.0
      z: 0.0
    - x: 22.0
      y: 0.0
      z: 0.0
    - x: 22.0
      y: 1.0
      z: 0.0
    - x: 20.0
      y: 1.0
      z: 0.0"

rostopic pub -1 /send_hull_info geometry_msgs/Polygon "points:
- x: 0.0
  y: 0.0
  z: 0.0
- x: 20.0
  y: 0.0
  z: 0.0
- x: 20.0
  y: 15.0
  z: 0.0
- x: 0.0
  y: 15.0
  z: 0.0"
