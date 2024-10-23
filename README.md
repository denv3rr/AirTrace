<a href="https://seperet.com">
<img src="https://user-images.githubusercontent.com/74038190/212284100-561aa473-3905-4a80-b561-0d28506553ee.gif">
</a>

<div align="center">

  <br></br>
  <br></br>
  <div>      

  ### **AirTrace**
  
  ![GitHub top language](https://img.shields.io/github/languages/top/denv3rr/AirTrace)
  ![CMake Min Version Required](https://img.shields.io/badge/cmake_min_vers_req-3.10-green)
  ![GitHub Created At](https://img.shields.io/github/created-at/denv3rr/AirTrace)
  
  ![GitHub repo size](https://img.shields.io/github/repo-size/denv3rr/AirTrace)
  ![GitHub last commit (branch)](https://img.shields.io/github/last-commit/denv3rr/AirTrace/main)
  ![Website](https://img.shields.io/website?url=https%3A%2F%2Fseperet.com)

  <br></br>
  <br></br>
    <img src="https://user-images.githubusercontent.com/74038190/212284100-561aa473-3905-4a80-b561-0d28506553ee.gif">
    <br></br>
[About](#adaptable) | [Run and Debug](#run-and-debug) | [Issues](#issues) | [Contributing](#contributing) | [Links](#links) | [Contact](#contact)
    <br></br>
  <a href="https://seperet.com">
    <img src="https://user-images.githubusercontent.com/74038190/212284100-561aa473-3905-4a80-b561-0d28506553ee.gif">
  </a>      
</div>

<br></br>
<br></br>

### **Adaptable.**

A **tracking solution** designed for dynamic environments with precision via automated mode-switching - reliability even in the event of a sensor failure or error.

This is (*currently*) a **console application** being built to research movement patterns, Kalman filtering, heat signature detection, and predictive modeling.

<br></br>
<br></br>
<a href="https://seperet.com" align="center">
    <img src="https://user-images.githubusercontent.com/74038190/212284100-561aa473-3905-4a80-b561-0d28506553ee.gif" width="300">
</a>

<br></br>
<div align="left">
  
**Built with:**

![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white) ![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white)
<br></br>

**Key Features (Planned):**
<details><summary>Expand</summary>

  - **Kalman Filtering:** Leverages a few statistical methods for optimal state estimation, ensuring accurate position tracking even in noisy conditions.
   
  - **Heat Signature Detection:** Integrates thermal data to enhance tracking capabilities, enabling the identification of objects based on their heat emissions.
   
  - **Predictive Algorithms:** Employ quick prediction techniques to anticipate movements and improve response times in any potential scenarios.
    
  - **Dead Reckoning Failsafe:** Maintains tracking integrity when GPS or thermal inputs are compromised, ensuring continuous operation through smart estimations of position.
    
  - **Real-Time and Test Modes:** Easily switch between real-time tracking and a comprehensive test mode for simulations and performance evaluations, facilitating robust development and debugging.   

  - **Built with CMake** for modularity and scalability, this application is designed for developers looking to integrate sophisticated tracking capabilities into their projects.

</details>

</div>

<br></br>
<br></br>
<br></br>

<img src="https://user-images.githubusercontent.com/74038190/212284100-561aa473-3905-4a80-b561-0d28506553ee.gif">

### **Run and Debug**

<details><summary>Expand</summary>

<br></br>

<div align="left">

1. **Clone source code or download the `.zip`**

    - Clone: `git clone https://github.com/denv3rr/AirTrace`
    - `.zip`: https://github.com/denv3rr/AirTrace/archive/refs/heads/main.zip
   
2. **Build and run** (as of 10/23/2024)
   
    - **To build using CMake:**
  
        - Make sure you have CMake (3.10+): https://cmake.org/cmake/download
        - Navigate to `AirTrace\build`
        - Run cmake: `cmake ..`
        - Build: `cmake --build .`
        - The project is built. 
  
     - **To run from the `Debug` folder:**
  
        - `cd` from `AirTrace\build` to: `AirTrace\build\Debug`
        - Run the `AirTrace.exe` file that you built.

        -  ````
            > .\AirTrace.exe
           
            AirTrace

            Main Menu: Select Mode
            
            1. Real Input Mode
            2. Test Mode (Manual Configuration)
            3. View and Rerun Previous Simulations
            4. Delete a Previous Simulation
            5. Exit
            
            Select an option:
          
            ````
          - Select option `2`.
          - ````
            --------------------------------------------


            Testing and Debugging Menu
            
            NOTE: YOU ARE IN 'TEST MODE'
            The 'scenario mode' is being designed for dynamic switching
            between these modes automatically based on external input.
            
            1. Prediction
            2. Kalman Filter
            3. Heat Signature
            4. GPS
            5. Dead Reckoning
            
            Select a tracking mode:
            
            ````
          - Selection `3` output example:
          - ````
            Select a tracking mode: 3

            --------------------------------------------

            Follower updated to position: (60, 33)
            
            [Iteration 0]
            --------------------------------------------
            Target Position: (43, 90)
            Follower Position: (60, 33)
            Distance to Target: 59.48 units
            Heat Signature: 1.65 units
            --------------------------------------------
            Follower updated to position: (60, 33)

            [Iteration 1]
            --------------------------------------------
            ...
            ...
            <Updates here>
            ...
            ...
            
            ````
</div>
</details>
<br></br>
<br></br>
<br></br>

<img src="https://user-images.githubusercontent.com/74038190/212284100-561aa473-3905-4a80-b561-0d28506553ee.gif">


### **Issues**

<details>
<summary>Expand</summary>

<br></br>

If you have any problems with anything, **submit an issue** to let me know.

[AirTrace Issues](https://github.com/denv3rr/AirTrace/issues)

</details>

<br></br>
<br></br>
<br></br>

<img src="https://user-images.githubusercontent.com/74038190/212284100-561aa473-3905-4a80-b561-0d28506553ee.gif">

### **Contributing**

<details>
<summary>Expand</summary>

<br></br>

Contributions are welcome.

Feel free to fork this repo and create a pull request with any changes.

[Create a new fork](https://github.com/denv3rr/AirTrace/fork)

</details>
<br></br>
<br></br>
<br></br>
<img src="https://user-images.githubusercontent.com/74038190/212284100-561aa473-3905-4a80-b561-0d28506553ee.gif">

### **Links**

<details>
<summary>Expand</summary>
<br></br>

<div align="left">
  
- **Seperet** || [Seperet](https://seperet.com) ![Website](https://img.shields.io/website?url=https%3A%2F%2Fseperet.com)  
<img src="https://user-images.githubusercontent.com/74038190/212284100-561aa473-3905-4a80-b561-0d28506553ee.gif" width="300">
<br></br>
  
- **Kalman Filter** || [An Introduction to the Kalman Filter - MIT](http://www.mit.edu/~jwilson/kalman.pdf)
- **Understanding Kalman Filters** || [An Introduction to the Kalman Filter - University of North Carolina](https://www.cs.unc.edu/~welch/media/pdf/kalman_intro.pdf)
- **Tutorial: The Kalman Filter** || [Tutorial: The Kalman Filter - MIT](https://web.mit.edu/kirtley/kirtley/binlustuff/literature/control/Kalman%20filter.pdf)
- **Predictive Algorithms in Motion Tracking** || [An Elementary Introduction to Kalman Filtering - University of Texas](https://www.cs.utexas.edu/~mooney/cs343/kalman.pdf)
- **Thermal Detection** || [Long-Range Thermal Target Detection in Data-Limited Settings Using Restricted Receptive Fields](https://www.mdpi.com/1424-8220/23/18/7806) 
- **Detecting Minimal Thermal Signatures** || [Detecting Small Size and Minimal Thermal Signature Targets in Infrared Imagery Using Biologically Inspired Vision](https://www.mdpi.com/1424-8220/21/5/1812)

</div>
</details>

<br></br>
<br></br>
<br></br>
<img src="https://user-images.githubusercontent.com/74038190/212284100-561aa473-3905-4a80-b561-0d28506553ee.gif">

### **Contact**
<br></br>
  <img href="https://seperet.com/contact" src="https://user-images.githubusercontent.com/74038190/216120981-b9507c36-0e04-4469-8e27-c99271b45ba5.png" width="200" height="200">
  <br></br>
  <a href="https://seperet.com/contact"><img src="https://readme-typing-svg.demolab.com?font=Sixtyfour+Convergence&size=25&duration=3000&pause=500&color=F7F7F7&center=true&width=520&height=60&lines=CLICK+HERE;TO+CONTACT" alt="Typing SVG" /></a>
<br></br>
<br></br>
<br></br>

<img src="https://user-images.githubusercontent.com/74038190/212284100-561aa473-3905-4a80-b561-0d28506553ee.gif">

<br></br>
<div align="center">
    <a href="https://seperet.com">
        <img src="https://images.squarespace-cdn.com/content/v1/612e9679ca3ada2f6398ebb3/5c5d2752-5b10-4535-9591-acff011443fa/3dgifmaker21328.gif?format=1500w" width="300" height="300"/>
    </a>
</div>
<br></br>
<br></br>
<img src="https://user-images.githubusercontent.com/74038190/212284100-561aa473-3905-4a80-b561-0d28506553ee.gif">
