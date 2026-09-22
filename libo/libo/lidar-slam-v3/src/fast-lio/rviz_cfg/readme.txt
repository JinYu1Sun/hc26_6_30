rosrun rviz rviz -d $(rospack find fast_lio)/rviz_cfg/fusion_viz.rviz

<param name="enable_viz"   type="bool"   value="true" />   <!-- 关闭可视化设 false -->
<param name="viz_frame_id" type="string" value="map"  />
<param name="viz_path_max" type="int"    value="3000" />   <!-- 轨迹保留点数 -->


