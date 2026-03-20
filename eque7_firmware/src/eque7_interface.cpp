#include "eque7_firmware/eque7_interface.hpp"
#include <hardware_interface/types/hardware_interface_type_values.hpp>
#include <pluginlib/class_list_macros.hpp>

namespace eque7_firmware
{
Eque7Interface::Eque7Interface()
{
}


Eque7Interface::~Eque7Interface()
{
  if (arduino_.IsOpen())
  {
    try
    {
      arduino_.Close();
    }
    catch (...)
    {
      RCLCPP_FATAL_STREAM(rclcpp::get_logger("Eque7Interface"),
                          "Something went wrong while closing connection with port " << port_);
    }
  }
}


CallbackReturn Eque7Interface::on_init(const hardware_interface::HardwareInfo &hardware_info)
{
  CallbackReturn result = hardware_interface::SystemInterface::on_init(hardware_info);
  if (result != CallbackReturn::SUCCESS)
  {
    return result;
  }

  try
  {
    port_ = info_.hardware_parameters.at("port");
  }
  catch (const std::out_of_range &e)
  {
    RCLCPP_FATAL(rclcpp::get_logger("Eque7Interface"), "No Serial Port provided! Aborting");
    return CallbackReturn::FAILURE;
  }

  // Reserve memory for joint data (usually 2 joints = left/right wheel)
  velocity_commands_.reserve(info_.joints.size());
  position_states_.reserve(info_.joints.size());
  velocity_states_.reserve(info_.joints.size());
  // Save the current time for later (used for delta time)
  last_run_ = rclcpp::Clock().now();

  return CallbackReturn::SUCCESS;
}


std::vector<hardware_interface::StateInterface> Eque7Interface::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> state_interfaces;

  
  for (size_t i = 0; i < info_.joints.size(); i++)
  {
    // Position = total wheel rotation in radians
    state_interfaces.emplace_back(hardware_interface::StateInterface(
        info_.joints[i].name, hardware_interface::HW_IF_POSITION, &position_states_[i]));
    // Velocity = wheel angular speed in rad/s    
    state_interfaces.emplace_back(hardware_interface::StateInterface(
        info_.joints[i].name, hardware_interface::HW_IF_VELOCITY, &velocity_states_[i]));
  }
  // These will be READ by diff_drive_controller
  return state_interfaces;
}


std::vector<hardware_interface::CommandInterface> Eque7Interface::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;

  // Provide only a velocity Interafce
  for (size_t i = 0; i < info_.joints.size(); i++)
  {
    command_interfaces.emplace_back(hardware_interface::CommandInterface(
        info_.joints[i].name, hardware_interface::HW_IF_VELOCITY, &velocity_commands_[i]));
  }
  // diff_drive_controller will WRITE to velocity_commands_[i]
  return command_interfaces;
}


CallbackReturn Eque7Interface::on_activate(const rclcpp_lifecycle::State &)
{
  RCLCPP_INFO(rclcpp::get_logger("Eque7Interface"), "Starting robot hardware ...");

  // Reset commands and states
  velocity_commands_ = { 0.0, 0.0 };
  position_states_ = { 0.0, 0.0 };
  velocity_states_ = { 0.0, 0.0 };

  try
  {
    arduino_.Open(port_);
    arduino_.SetBaudRate(LibSerial::BaudRate::BAUD_115200);
  }
  catch (...)
  {
    RCLCPP_FATAL_STREAM(rclcpp::get_logger("Eque7Interface"),
                        "Something went wrong while interacting with port " << port_);
    return CallbackReturn::FAILURE;
  }

  RCLCPP_INFO(rclcpp::get_logger("Eque7Interface"),
              "Hardware started, ready to take commands");
  return CallbackReturn::SUCCESS;
}


CallbackReturn Eque7Interface::on_deactivate(const rclcpp_lifecycle::State &)
{
  RCLCPP_INFO(rclcpp::get_logger("Eque7Interface"), "Stopping robot hardware ...");

  if (arduino_.IsOpen())
  {
    try
    {
      arduino_.Close();
    }
    catch (...)
    {
      RCLCPP_FATAL_STREAM(rclcpp::get_logger("Eque7Interface"),
                          "Something went wrong while closing connection with port " << port_);
    }
  }

  RCLCPP_INFO(rclcpp::get_logger("Eque7Interface"), "Hardware stopped");
  return CallbackReturn::SUCCESS;
}


hardware_interface::return_type Eque7Interface::read(const rclcpp::Time &,
                                                          const rclcpp::Duration &)
{
  // Interpret the string
  if(arduino_.IsDataAvailable())
  {
    // Compute elapsed time (dt)
    auto dt = (rclcpp::Clock().now() - last_run_).seconds();

    std::string message;
    arduino_.ReadLine(message); // e.g. "rp7.23,ln6.45,"  (r=right, l=left, p/n=sign)
    std::stringstream ss(message);
    std::string res;
    int multiplier = 1;

    // Split by comma
    while(std::getline(ss, res, ','))
    {
      // Determine sign
      multiplier = res.at(1) == 'p' ? 1 : -1;

      // Right wheel data
      if(res.at(0) == 'r')
      {
        velocity_states_.at(0) = multiplier * std::stod(res.substr(2, res.size()));
        position_states_.at(0) += velocity_states_.at(0) * dt;
      }
      // Left wheel data
      else if(res.at(0) == 'l')
      {
        velocity_states_.at(1) = multiplier * std::stod(res.substr(2, res.size()));
        position_states_.at(1) += velocity_states_.at(1) * dt;
      }
    }
    last_run_ = rclcpp::Clock().now();
  }
  return hardware_interface::return_type::OK;
}


hardware_interface::return_type Eque7Interface::write(const rclcpp::Time &,
                                                          const rclcpp::Duration &)
{
  // Implement communication protocol with the Arduino
  std::stringstream message_stream;
  char right_wheel_sign = velocity_commands_.at(0) >= 0 ? 'p' : 'n';
  char left_wheel_sign = velocity_commands_.at(1) >= 0 ? 'p' : 'n';

  // Add leading zero if less than 10 for consistent string length (eg. 8.17 > 08.17, 12.17 remain 12.17)
  std::string compensate_zeros_right = "";
  std::string compensate_zeros_left = "";
  if(std::abs(velocity_commands_.at(0)) < 10.0)
  {
    compensate_zeros_right = "0";
  }
  else
  {
    compensate_zeros_right = "";
  }
  if(std::abs(velocity_commands_.at(1)) < 10.0)
  {
    compensate_zeros_left = "0";
  }
  else
  {
    compensate_zeros_left = "";
  }
  
  // Construct message string like: "rp07.23,ln06.45,"
  message_stream << std::fixed << std::setprecision(2) << 
    "r" << right_wheel_sign << compensate_zeros_right << std::abs(velocity_commands_.at(0)) << 
    ",l" <<  left_wheel_sign << compensate_zeros_left << std::abs(velocity_commands_.at(1)) << ",";

  try
  { 
    RCLCPP_INFO(rclcpp::get_logger("Eque7Interface"),"Sending to Arduino: %s", message_stream.str().c_str());
    arduino_.Write(message_stream.str());
  }
  catch (...)
  {
    RCLCPP_ERROR_STREAM(rclcpp::get_logger("Eque7Interface"),
                        "Something went wrong while sending the message "
                            << message_stream.str() << " to the port " << port_);
    return hardware_interface::return_type::ERROR;
  }

  return hardware_interface::return_type::OK;
}
}  // namespace eque7_firmware

PLUGINLIB_EXPORT_CLASS(eque7_firmware::Eque7Interface, hardware_interface::SystemInterface)