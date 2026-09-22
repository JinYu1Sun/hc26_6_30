source devel/setup.bash 

echo "Select the mode"
echo "1. Create map mode"
echo "2. Location mode"
read -p "Enter your choice: " choice

if ((choice == 1)); then
     roslaunch fast_lio create_mode.launch
elif ((choice == 2)); then
     roslaunch fast_lio location_mode.launch
else
    echo "Invalid choice!"
fi

# 由android端启动控制
#roslaunch android_manager android.launch > /dev/null 2>&1 &
