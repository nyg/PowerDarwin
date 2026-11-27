# iMac G5 (iSight), PowerMac12,1

Tested: PowerDarwin reaches Steps 1 to 10 and 13 on this model, and runs `awk`, `tar`, `gzip`, `file`, `lsof`, `top` and `curl`, built after Step 10. A release image, restored by hand, asks for root's password on the console at its first boot and turns SSH on after it. Wi-Fi, graphics acceleration and sleep do not work, since their drivers are closed or not yet replaced. The optical drive did not see a DVD even under Leopard, so CDs and DVDs were tested through a USB drive. FireWire has our `OpenFWOHCI` in place of the closed `AppleFWOHCI`, and its bus comes up with no device attached; no FireWire device has been tested yet.

## Drivers

The drivers Leopard 10.5.8 loads on this model, matched to the 9L30 projects that build them. Raw data: [reference/imacg5/](../reference/imacg5/).

A driver's `CFBundleVersion` encodes its project version: `2.0.4d9` is project 204.0.9, `1.7.2f1` is 172.3.1. Every driver below whose project is listed carries exactly that project's 9L30 version, except `AppleGPIO` and `MacIOGPIO` (`1.3.0d0`, while 9L30 lists `AppleGPIO` 131.0.0).

### Needed for a single-user shell

| Drivers | 9L30 project | Source |
|---|---|---|
| `System.kext` and its plug-ins | `xnu` 1228.15.4 | open |
| `AppleMacRISC4PE` (platform) | `AppleMacRISC4PE` 204.0.9 | open; builds with 66 register constants that a patch defines ([D39](../DEVIATIONS.md#d39)); the published source also leaves parts of five functions out, of which only the model name in panic logs applies to the iMac G5 |
| `IOPlatformFunction` | `AppleMacRISC2PE` 187.0.5 | open |
| `AppleMPIC` (interrupts) | `AppleMPIC` 1.5.3 | open |
| `IOPCIFamily`, `AppleMacRiscPCI` | `IOPCIFamily` 110, `AppleMacRiscPCI` 3.4 | open |
| `IOKeyLargo`, `AppleK2`, `AppleK2Driver` (I/O controller) | `AppleKeyLargo` 172.3.1 | open |
| `AppleK2SATARoot`, `AppleK2SATA` (internal disk) | `AppleK2SATARoot` 105.2.2, `AppleK2SATA` 104.3.2 | open |
| `IOATAFamily`, `IOATABlockStorage`, `IOATAPIProtocolTransport` | `IOATAFamily` 202.0.1, `IOATABlockStorage` 200.1.8, `IOATAPIProtocolTransport` 153.0.1 | open, except `IOATABlockStorage`: never published, the last published is Panther's 130.3.1, which we build in its place ([D40](../DEVIATIONS.md#d40)) |
| `IOStorageFamily` | `IOStorageFamily` 92.9 | open |
| `IOGraphicsFamily`, `IONDRVSupport` (screen through the firmware's display driver) | `IOGraphics` 305.14 | open; marked `Safe Boot` and absent from the reference system's `Extensions.mkext`, so a single-user shell keeps the firmware's framebuffer without them; Apple's `IONDRVSupport` also holds `IOATINDRV` and `IOATI128NDRV`, which are not published |
| `IOUSBFamily` with `AppleUSBOHCI`, `AppleUSBEHCI`, `AppleUSBHub`, `IOUSBCompositeDriver`, `IOUSBHIDDriver`; `IOHIDFamily` (USB keyboard) | `IOUSBFamily` 349.4.3, `IOHIDFamily` 258.14 | open |
| `AppleSMU` 1.2.1d1 (system management unit) | none; Tiger's 8S165 lists `AppleSMU` 106.0.3 | closed: the shell boots without it on the iMac, but the kernel then has no clock (`RTC did not show up`) and cannot restart or power off the iMac (`MACH Reboot`, then nothing); our `OpenSMU` ([D81](../DEVIATIONS.md#d81)) stands in for the clock, restart and power-off, not for its fan, sensor and I²C nubs, sleep or wake events |
| `IOPlatformPluginFamily` with `PowerMac12_1_PlatformPlugin`, `SMU_Neo2_PlatformPlugin` (fans and temperatures) | none | closed: without them the SMU's firmware runs the fans on its own, noisily; our `OpenSMU` drives the fans instead, from the same sensors ([D81](../DEVIATIONS.md#d81)) |

### Not needed for a shell

Open: `AppleSMUMonitor`, `IOI2CLM7x`, `IOI2CMaxim6690` (`AppleHWSensor` 190); `IOI2CFamily`, `IOI2CControllerPPC`, `IOI2CControllerSMU` (`IOI2CFamily` 111.0.2); `AppleI2C`; `AppleGPIO`, `MacIOGPIO`; `AppleFlashNVRAM`; `AppleKauaiATA` (optical drive bus); `IOCDStorageFamily`, `IODVDStorageFamily`, `IOBDStorageFamily`; `IONetworkingFamily` with `AppleGMACEthernet` (Ethernet); `IOFireWireFamily`, `IOFireWireIP`; `AppleI2S`, `AppleOnboardAudio` with `AppleDBDMAAudio`, `IOAudioFamily` (whose one call into the closed `OSvKernDSPLib` is replaced, [D87](../DEVIATIONS.md#d87)); `IOSerialFamily`; `IOUSBMassStorageClass` (USB disks and drives, [D86](../DEVIATIONS.md#d86)); `BootCache`; `autofs`.

Closed or not in 9L30: `IOSCSIArchitectureModelFamily` 200.3.6 with `IOSCSIBlockCommandsDevice`, `IOSCSIMultimediaCommandsDevice` and `SCSITaskUserClient` (never published, the last published is Panther's 139.0.2, which we build in its place, [D85](../DEVIATIONS.md#d85)), `AppleFWOHCI` (the FireWire controller driver, a plug-in of `IOFireWireFamily.kext` that neither its source nor any IOFireWireFamily on Apple's GitHub holds; our `OpenFWOHCI` stands in for it, [D119](../DEVIATIONS.md#d119), and IOFireWireFamily's own `IOFWSimplePhysicalAddressSpace`, whose published source is empty from 344.4.0 on, is ours too, [D120](../DEVIATIONS.md#d120)), `ATIRadeon9700` (graphics acceleration, and the GPU's power settings, which our `OpenRadeonPM` sets instead: clock gating and the PCI Express link's L0s state, [D117](../DEVIATIONS.md#d117)), `AppleOnboardDisplay`, `AppleIRController`, `IO80211Family` with `AppleAirPortBrcm4311` (AirPort Extreme, a built-in Broadcom BCM4318, `pci14e4,4318`; the book's [Wi-Fi: what it would take](../docs/src/synthesis/wifi.md) looks at an open driver for it), `IOBluetoothFamily` and its plug-ins, `IOHDIXController` (disk images), `ALF` (firewall), `AppleXsanFilter`, `TMSafetyNet`, `seatbelt` (sandbox), `OSvKernDSPLib`, `AudioIPCDriver`, `IOI2CPulsar`, `CHUDKernLib`, `CHUDProf` and `CHUDUtils` (installed with Xcode's CHUD tools, not with Leopard), `AppleOnyxAudio` (AppleOnboardAudio's plug-in for the iMac G5's PCM3052 codec, published only as a stub source; our `OpenOnyx` stands in for it, [D94](../DEVIATIONS.md#d94)).

On the image but not loaded here: our `OpenPMU` ([D118](../DEVIATIONS.md#d118)), the stand-in for `ApplePMU`, matches only the `pmu` nub that `AppleVIA` makes for a `via-pmu` node. This model has an SMU and no `via-pmu` node, and Leopard loads neither `AppleVIA` nor `ApplePMU` on it ([reference/imacg5/](../reference/imacg5/)).
