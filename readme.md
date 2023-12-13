#### Getting started with development:
- Make sure platfromio is installed, to do this
1. Go inside root of project
2. Run, `poetry shell` (as this is using poetry to setup platfromio)
3. Run, `poetry install`
4. Run, `platformio run --target upload`

```bash
poetry shell
poetry install
platformio run --target upload
```

#### Other command:
- See avalialble tty devices
    - ls -tr1 /dev/tty*
- Allowing dev devices, use whichever needed 
    - (Arch linux) sudo usermod -a -G uucp $USER 
    - sudo usermod -a -G dialout $USER
    - sudo chmod 777 /dev/ttyUSB0
    - sudo chmod 777 /dev/ttyACM0

#### Opening Serial port
- Install minincom, `sudo pacman -S minicom` 
- minicom -D /dev/ttyUSB0 -b 9600
