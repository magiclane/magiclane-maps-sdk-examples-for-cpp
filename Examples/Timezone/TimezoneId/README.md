## Overview

This example app demonstrates the following features:
- Get information about timezone from specific timezone ID ("Continent/City") and a timestamp.

## How to use the sample

When you run the example app, the `TimezoneResult` is filled with information about the timezone. The result (timezone id, UTC / DST / current offsets) is printed to the SDK log on success, and the error codes are logged on failure; the same message goes to stdout when run from a console.

## How it works

1. Create a `ProgressListener`, `TimezoneService`, timezone id and the current time, using `gem::Time::getUniversalTime()` (note: `gem::Time` is in milliseconds).
2. Create a `TimezoneResult`, which is the output for info.
3. Call the `getTimezoneInfo` method from the `TimezoneService` using the timezoneId and timestamp from 1.) as input and `TimezoneResult` from 2.) as output and `ProgressListener`.
4. Once the operation completes, the `TimezoneResult` from 2.) will be populated.
