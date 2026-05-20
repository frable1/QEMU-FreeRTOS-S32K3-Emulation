#   **01GYKUV - Computer Architectures and Operating Systems** project - Group 17 - FreeRTOS + QEMU on S32K3X8EVB

This project provides a FreeRTOS demo for S32K3X8EVB board, built and tested using QEMU.

Group 17 composed by:
- Ble Francesco, s343431@studenti.polito.it
- Cocca Donatella, s347942@studenti.polito.it
- Crudele Antonio, s338641@studenti.polito.it
- Dokollari Megi, s346293@studenti.polito.it
- Frisenda Giovanni, s337127@studenti.polito.it

Group peripherals assignment: UART and GPIO

## Features

1.  FreeRTOS basic demo
2.  UART printing
3.  GPIO simulation with debouncing
4.  MPU initialization, self-test and forced fault demo

##  Installation and Emulation

1.  Clone the repository

    ```sh
    git clone --recurse-submodules https://baltig.polito.it/caos2024/group17.git 
    cd group17
    ```

2.  Configure and build QEMU (some packages are needed, look in the [GUIDE](./GUIDE.md))
    ```sh
    mkdir build
    cd build
    ../qemu/configure
    make -j
    ```

3.  Run the application
    ```sh
    cd demo
    make
    make qemu_start
    ```



##  Guide for recreating the Project
All the details about how to set up, run and more generalically recreate the entire project, included the steps followed are contained in the file [GUIDE.md](./GUIDE.md).

## License

This project is licensed under the [Creative Commons Attribution 4.0 International License](./LICENSE.md).
