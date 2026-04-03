

# A Comparative Study: LiDAR, Visual & LiDAR-Visual Fusion SLAM for Low Cost Indoor Mobile Robot

![Status](https://img.shields.io/badge/Status-Work_in_Progress-orange)


> **Note:** This repository contains the ongoing work for my Final Year Project (FYP). It serves as a practical evaluation of different SLAM modalities on a low-cost robotic platform.

## Overview
As mobile robots increasingly navigate GNSS-denied indoor environments, selecting the optimal Simultaneous Localization and Mapping (SLAM) algorithm is critical. This project evaluates and compares LiDAR-based SLAM, RGB-D Visual SLAM, and LiDAR-Visual Fusion SLAM operating on a custom differential drive robot. 

The study focuses on the real-world performance of consumer-grade sensors, analyzing trajectory accuracy, map quality, and computational resource utilization using a decoupled data-collection and offline-processing architecture.

![Eque7-Bot](https://github.com/user-attachments/assets/60bd2d33-c1d8-4619-bc9b-75bac3345ad7)

## System Configuration
### Hardware
The system is divided into an edge data-collection (the robot) and an offline processing (a laptop).
* **Robot Compute (Data Collection):** Raspberry Pi 4B (8GB RAM)
* **Offline Processing (SLAM Execution):** Personal Laptop
* **2D LiDAR:** RPLidar A2M8
* **RGB-D Camera:** Intel RealSense D415
* **IMU:** MPU-6050
* **Motor:** 12V DC Motor
### Software

* ROS 2 Humble
* `slam_toolbox`
* `rtabmap_ros`
* EVO toolkit (trajectory evaluation)
* Foxglove & RViz (visualization)

## Project Workflow
The evaluation is conducted systematically through the following pipeline:
1. **Robot Setup & Sensor Integration:** Assembling hardware, tuning motor PID controllers, setting up sensor fusion (IMU + wheel encoders), and integration of the 2D LiDAR and camera.
2. **Environmental Setup:** Creating physical markers on the floor of the test environment to establish a sparse ground truth coordinate system.
3. **Data Collection:** Teleoperating the robot using the Raspberry Pi 4B to record essential ROS 2 topics into a `rosbag`.
4. **Offline SLAM Execution:** Playing back the `rosbag` on a laptop to run SLAM algorithms deterministically.
5. **Data Conversion:** Extracting the generated robot poses from the `/tf` tree and converting them into the standard TUM format.
6. **Performance Evaluation:** Analyzing the resulting trajectories using the `evo` toolkit, evaluating the 2D occupancy maps in RViz, and profiling computational usage.

## Evaluated SLAM Algorithms
-  **LiDAR SLAM:** [SLAM Toolbox](https://github.com/SteveMacenski/slam_toolbox)
- **Visual SLAM:** [RTAB-Map (RGB-D Mode)](https://github.com/introlab/rtabmap_ros)
- **Fusion SLAM:** RTAB-Map (RGB-D + LiDAR Scan Mode) 

## Sparse Ground Truth Methodology
Since standard indoor environments lack access to GNSS or precise motion capture systems, a Sparse Ground Truth methodology was adopted for trajectory evaluation:
* **Reference Points:** Several reference points are manually marked on the floor of the test environment. Their exact (X, Y) coordinates are measured relative to the robot's starting location using a tape measure.
* **Data Collection Pause:** During teleoperation, the robot is driven to each reference point and kept completely stationary for 5 seconds. This ensures the robot's pose is stable and easily identifiable in the data stream.
* **Timestamp Matching:** Post-experiment, the timestamps corresponding to these 5-second stationary moments are extracted from the recorded trajectories.
* **Evaluation:** The known, measured coordinates are matched with these timestamps to form a sparse ground truth trajectory. Localization accuracy is then evaluated by calculating the Absolute Trajectory Error (ATE) between the SLAM-estimated poses and the measured waypoints.


## Current Progress
- ✅ Initial system modeling and algorithm testing in Gazebo simulation.
- ✅ Hardware integration, motor PID tuning, and IMU/Encoder sensor fusion.
- ✅ ROS 2 sensor bring-up (RPLidar & RealSense drivers).
- ✅ **Pipeline Validation (Small Test Room):** Successfully collected `rosbag` data (`/tf`, `/odom`, `/scan`, `/camera/...`) to verify hardware reliability.
- ✅ **Pipeline Validation (Small Test Room):** Successfully verified the offline SLAM execution (SLAM Toolbox, RTAB-Map) and `evo` toolkit evaluation workflow on the test data.
- ✅ TF-to-TUM data conversion pipeline fully established.
- ⏳ **Pending:** Full data collection using the sparse ground truth method in the final experimental environment.
- ⏳ **Pending:** Final benchmarking data compilation, visual map inspection, and computational load analysis.

