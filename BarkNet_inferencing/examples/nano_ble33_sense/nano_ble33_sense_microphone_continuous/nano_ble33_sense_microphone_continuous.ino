/* Edge Impulse ingestion SDK
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// If your target is limited in memory remove this macro to save 10K RAM
#define EIDSP_QUANTIZE_FILTERBANK   0

/**
 * Define the number of slices per model window. E.g. a model window of 1000 ms
 * with slices per model window set to 4. Results in a slice size of 250 ms.
 * For more info: https://docs.edgeimpulse.com/docs/continuous-audio-sampling
 */
#define EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW 4

/*
 ** NOTE: If you run into TFLite arena allocation issue.
 **
 ** This may be due to may dynamic memory fragmentation.
 ** Try defining "-DEI_CLASSIFIER_ALLOCATION_STATIC" in boards.local.txt (create
 ** if it doesn't exist) and copy this file to
 ** `<ARDUINO_CORE_INSTALL_PATH>/arduino/hardware/<mbed_core>/<core_version>/`.
 **
 ** See
 ** (https://support.arduino.cc/hc/en-us/articles/360012076960-Where-are-the-installed-cores-located-)
 ** to find where Arduino installs cores on your machine.
 **
 ** If the problem persists then there's not enough memory for this model and application.
 */

/* Includes ---------------------------------------------------------------- */
#include <PDM.h>
#include <BarkNet_inferencing.h>
#include <ArduinoBLE.h>


/** Audio buffers, pointers and selectors */
typedef struct {
    signed short *buffers[2];
    unsigned char buf_select;
    unsigned char buf_ready;
    unsigned int buf_count;
    unsigned int n_samples;
} inference_t;

static inference_t inference;
static bool record_ready = false;
static signed short *sampleBuffer;
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
static int print_results = -(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW);

bool detect_barks = false; // Detection flag
char detectionMode = 'p'; // Detection mode variable, default to piezo buzzer mode

//*************************************************
//Bluetooth BLE service
BLEService barkService("12345678-1234-1234-1234-123456789abc");
BLEStringCharacteristic barkCharacteristic("12345678-1234-1234-1234-123456789def", BLERead | BLENotify, 100); 
//*************************************************

/**
 * @brief      Function to clear serial monitor
 */
void clearSerial() {
    while (Serial.available() > 0) {
      Serial.read(); // Discard anything and everything in the serial console
    }
}

/**
 * @brief      Arduino setup function
 */
void setup()
{
    // initialize digital pin LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    
    // put your setup code here, to run once:
    Serial.begin(115200);

    // Clear serial output
    clearSerial();

    digitalWrite(LED_BUILTIN, LOW); // default state should be off

    // comment out the below line to cancel the wait for USB connection (needed for native USB)
    while (!Serial);
    Serial.println("==================================");
    Serial.println("BarkNet Inferencing Demo");
    Serial.println("   / \\__");
    Serial.println("  (    @\\___");
    Serial.println("  /         O");
    Serial.println(" /   (_____/");
    Serial.println("/_____/   U");
    Serial.println("==================================");
    Serial.println("");
    Serial.println("Available commands:");
    Serial.println("  detect - Initiate bark detection");
    Serial.println("  stop   - Stop bark detection");
    Serial.println("----------------------------------");

    run_classifier_init();
    if (microphone_inference_start(EI_CLASSIFIER_SLICE_SIZE) == false) {
        ei_printf("ERR: Could not allocate audio buffer (size %d), this could be due to the window length of your model\r\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT);
        return;
    }
}

/**
 * @brief      Arduino main function. Runs the inferencing loop.
 */
void loop()
{

    // Check for incoming serial commands
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        command.trim(); // Remove any leading/trailing whitespace

        if (command == "detect") {
            detect_barks = true;
            Serial.println("==================================");
            Serial.println("Bark detection ENABLED.");
            Serial.println("==================================");
            Serial.println("Available bark detection modes:");
            Serial.println("  p - Piezo buzzer (controlled by pin 8) will sound every time a bark is detected");
            Serial.println("  l - LED will blink every time a bark is detected");
            Serial.println("  b - Bluetooth mode enabled. In this mode, bark detection notification will be sent to a central device");
        } else if (command == "stop") {
            detect_barks = false;
            Serial.println("==================================");
            Serial.println("Bark detection DISABLED.");
            Serial.println("==================================");
            Serial.println("Available commands:");
            Serial.println("  detect - Initiate bark detection");
            Serial.println("  stop   - Stop bark detection");
            Serial.println("----------------------------------");
        } else if (detect_barks && (command == "p" || command == "l" || command == "b")) {
            detectionMode = command.charAt(0);
            Serial.print("Detection mode: ");
            Serial.println(detectionMode);

            //*************************************************
            //Enable Bluetooth if b is selected by the user
            if (detectionMode == 'b') {
                if (!BLE.begin()) {
                    Serial.println("Starting BLE failed!");
                    return;
                }
                Serial.print("BLE MAC Address: ");
                Serial.println(BLE.address());
                
                BLE.setLocalName("BarkNet");
                BLE.setAdvertisedService(barkService);
                barkService.addCharacteristic(barkCharacteristic);
                BLE.addService(barkService);
                BLE.advertise();
                Serial.println("BLE Device Initialized");

                BLEDevice central = BLE.central();
                if (central) {
                    Serial.print("Connected to central: ");
                    Serial.println(central.address());

                    while (central.connected()) {
                        runInference();
                    }
                    Serial.print("Disconnected from central: ");
                    Serial.println(central.address());
            }

            }
            //*************************************************

        } else {
            Serial.println("Unknown command.");
        }
    }

    if (detect_barks) {
    //*************************************************
    //modify the Original code start Bluetooth connection only if user choose b(Bluetooth)
        if (detectionMode == 'b') {
            BLEDevice central = BLE.central();
            if (central) {
                Serial.print("Connected to central: ");
                Serial.println(central.address());

                while (central.connected()) {
                    runInference();
                }
                Serial.print("Disconnected from central: ");
                Serial.println(central.address());
            }
        } else {
            runInference();
        }
    }
}
//*************************************************


void runInference() {
    bool m = microphone_inference_record();
    if (!m) {
        // ei_printf("ERR: Failed to record audio...\n");
        return;
    }

    signal_t signal;
    signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
    signal.get_data = &microphone_audio_signal_get_data;
    ei_impulse_result_t result = {0};

    EI_IMPULSE_ERROR r = run_classifier_continuous(&signal, &result, debug_nn);
    if (r != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", r);
        return;
    }

    if (result.classification[1].value > 0.5) {
        switch (detectionMode) {
            case 'p':
                Serial.println("Bark Detected, sound the alarm!");
                // Use the tone function to emit a sound
                // tone() takes pin number, frequency in Hz, and duration in ms
                tone(8, 1000, 80);
                break;
            case 'l':
                Serial.println("Bark Detected, turn on the lights!");
                digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on 
                delay(1000);  // keep the LED on for x number of ms
                digitalWrite(LED_BUILTIN, LOW); 
                break;
            case 'b':
                Serial.println("Bark Detected, notify the central server!");
                barkCharacteristic.writeValue("Bark Detected!");
                break;
            default:
                Serial.println("Bark Detected");
                break;

        }
            
    }
    // Try not to overwhelm or spam the serial buffer by adding a delay
    delay(100);
    
}

    

/**
 * @brief      PDM buffer full callback
 *             Get data and call audio thread callback
 */
static void pdm_data_ready_inference_callback(void)
{
    int bytesAvailable = PDM.available();

    // read into the sample buffer
    int bytesRead = PDM.read((char *)&sampleBuffer[0], bytesAvailable);

    if (record_ready == true) {
        for (int i = 0; i<bytesRead>> 1; i++) {
            inference.buffers[inference.buf_select][inference.buf_count++] = sampleBuffer[i];

            if (inference.buf_count >= inference.n_samples) {
                inference.buf_select ^= 1;
                inference.buf_count = 0;
                inference.buf_ready = 1;
            }
        }
    }
}

/**
 * @brief      Init inferencing struct and setup/start PDM
 *
 * @param[in]  n_samples  The n samples
 *
 * @return     { description_of_the_return_value }
 */
static bool microphone_inference_start(uint32_t n_samples)
{
    inference.buffers[0] = (signed short *)malloc(n_samples * sizeof(signed short));

    if (inference.buffers[0] == NULL) {
        return false;
    }

    inference.buffers[1] = (signed short *)malloc(n_samples * sizeof(signed short));

    if (inference.buffers[1] == NULL) {
        free(inference.buffers[0]);
        return false;
    }

    sampleBuffer = (signed short *)malloc((n_samples >> 1) * sizeof(signed short));

    if (sampleBuffer == NULL) {
        free(inference.buffers[0]);
        free(inference.buffers[1]);
        return false;
    }

    inference.buf_select = 0;
    inference.buf_count = 0;
    inference.n_samples = n_samples;
    inference.buf_ready = 0;

    // configure the data receive callback
    PDM.onReceive(&pdm_data_ready_inference_callback);

    PDM.setBufferSize((n_samples >> 1) * sizeof(int16_t));

    // initialize PDM with:
    // - one channel (mono mode)
    // - a 16 kHz sample rate
    if (!PDM.begin(1, EI_CLASSIFIER_FREQUENCY)) {
        ei_printf("Failed to start PDM!");
    }

    // set the gain, defaults to 20
    PDM.setGain(127);

    record_ready = true;

    return true;
}

/**
 * @brief      Wait on new data
 *
 * @return     True when finished
 */
static bool microphone_inference_record(void)
{
    bool ret = true;

    if (inference.buf_ready == 1) {
        // ei_printf(
        //     "Error sample buffer overrun. Decrease the number of slices per model window "
        //     "(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW)\n");
        ret = false;
    }

    while (inference.buf_ready == 0) {
        delay(1);
    }

    inference.buf_ready = 0;

    return ret;
}

/**
 * Get raw audio signal data
 */
static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
    numpy::int16_to_float(&inference.buffers[inference.buf_select ^ 1][offset], out_ptr, length);

    return 0;
}

/**
 * @brief      Stop PDM and release buffers
 */
static void microphone_inference_end(void)
{
    PDM.end();
    free(inference.buffers[0]);
    free(inference.buffers[1]);
    free(sampleBuffer);
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
#error "Invalid model for current sensor."
#endif
