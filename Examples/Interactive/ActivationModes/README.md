## Overview

This example app demonstrates the following features:
- Every way an SDK with auto-activation (the SDKs downloaded from the Magic Lane website) can become, and stop being, activated.
- How an application can reflect the SDK's activation state to its user, here with a watermark text on the map.
- The manual offline activation and deactivation ceremony for devices that never go online, with the request blob shown as a QR code for the Magic Lane companion app and as the REST request a developer can send themselves.

## How to use the sample

The example starts with the internet connection **disallowed**, like a device without connectivity. Since the SDK cannot auto-activate, it reports itself as not activated and the map shows the watermark text `SDK not activated / Limited offline functionality`. The panel shows the activation state, the connection state, the application id, and the last notification received from the SDK, then offers four sections:

1. **Auto-activation (online)** - tick *Allow internet connection*. The SDK reaches Magic Lane Services, activates itself from the project token, reports `onSdkActivated`, and the watermark disappears. Untick it to go back to an offline device.
2. **Manual offline activation** - for a device that must never go online. Optionally enter a portal-issued license key (leave it empty to let the SDK generate one), then press *Get activation request blob*. The blob is produced entirely on-device and shown three ways: as a QR code to scan with the Magic Lane companion app, as raw text, and as the exact REST request you can send from any online machine. Both routes return an `offline_activation_key`; paste it and press *Complete offline activation*. The SDK becomes activated and the watermark disappears.
3. **Manual offline deactivation** - the mirror of the above, to release this device's activation (e.g. to decommission it or reuse its license key). Press *Get deactivation request blob* - the activation becomes pending deactivation at this point, so the SDK reports not activated and the watermark returns right away - take the blob to Magic Lane Services the same way, paste the returned `offline_deactivation_key` and press *Complete offline deactivation* to tidy up the record on this device.
4. **Reset to first run** - a testing aid: press *Reset to first run* to disallow the connection and delete this device's local activation records. The SDK reports not activated again, the watermark returns, and every scenario above can be replayed without restarting the example (allowing the connection runs auto-activation again). Only the local records are dropped - nothing is released at Magic Lane Services, so a real device must use the offline deactivation above.

## Building against an SDK built from source (Magic Lane developers)

The SDK built by the internal `3DNavigator` solution does **not** enable auto-activation by default, so this example would run against a plain on-demand activation SDK. Before building `ActivationModes` from the solution, add

```cpp
#define WITH_AUTO_ACTIVATION_FEATURE
```

to the generated `BUILD_WIN\3DNavigator\Generated\AppConfigurations.h` (created once by `GenerateAppConfigurations.bat` and then left untouched) and rebuild the `GEM` project. SDKs downloaded from the Magic Lane website already have it enabled.

## How it works

1. The activation-state notifications are registered with `Environment::SetActivationStateCallbacks()` **before** the SDK is initialized, because with a token but no connection the SDK already reports `onSdkNotActivated` during initialization. The callbacks may run on SDK threads, so they only record the state in atomics; the UI thread reacts to them each frame.
2. The SDK is initialized with `goOnline = false` so the connection starts disallowed; the panel toggles it with `gem::SdkSettings().setAllowConnection()`. Allowing it lets auto-activation run; the SDK then reports `onSdkActivated`.
3. The watermark is the application's own reaction to the notifications - the SDK draws nothing for this state. While not activated the example calls `mapView->setWatermarkText( u"SDK not activated", u"Limited offline functionality" )`, and clears it once activated.
4. The application id (the audience of the token) required by the offline calls is obtained with `gem::SdkSettings().getApplicationId()`, so the application does not need to keep the token around.
5. Manual offline activation uses `gem::ActivationService().getOfflineActivationRequestBlob()` to produce the request blob on-device (no network involved), and `completeOfflineActivation()` with the key returned by Magic Lane Services. Deactivation uses `getOfflineDeactivationRequestBlob()` with the license key of the active activation (read from `getActivationsForProduct()`) and `completeOfflineDeactivation()`.
6. The request blob is rendered as a QR code with the `nayuki-qr-code-generator` library for the companion app, and the equivalent REST request is displayed for developers who prefer to call the service themselves: `POST` (activation) or `DELETE` (deactivation) to `{activation service url}/services/tokens/v1/activations` with the JSON body `{"request_blob": "..."}`. The activation service URL comes from `gem::Debug().getDefUrls( 3 )` (3 is the activation service id), which the SDK only knows after it has received its services configuration from Magic Lane Services, i.e. after being online once. On a device that is never allowed online the panel offers a field to type in the URL and explains how to look it up from any online machine: `GET https://m71os.services.magicearthsdk.com/services_list_json` (the same list the SDK fetches) returns a JSON array of services, and the entry with `"service_id": 3` carries the activation service `"URLs"`.
7. The reset button disallows the connection and calls `gem::ActivationService().deleteActivation()` for every record returned by `getActivationsForProduct()`. Deleting the activation that holds the Core gate open is an activation-state transition like completing an (de)activation, so the SDK reports `onSdkNotActivated` immediately, and the next time the connection is allowed auto-activation runs again.
