# STM32F429 Discovery oscilloscope

### Great project, taken from https://mikrocontroller.bplaced.net/wordpress/?page_id=752
### Highlights:
- Ported to work in STM32CUBE IDE, configured in the way so CUBEMX does most of the initialization.

- The functionality is the same as original. 

- Flash usage is about 50% higher than the original, "thanks" to the HAL Library...

- The project builds right away. Just install STMCUBE IDE and import the project.

	#### PA5: Channel 1
	#### PA7: Channel 2
	#### PB2: 500Hz Test signal
	#### User "Blue" Button: Run / Stop / Single Trigger
	