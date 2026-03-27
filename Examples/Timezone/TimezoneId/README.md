## Overview

This example app demonstrates the following features:
- Get information about timezone from specific timezone ID ("Country/City") and timestamp.

## How to use the sample

When you run the example app, `TimezoneResult` field will be filled with information about timezone.

## How it works

1. Create a `ProgressListener`, `TimezoneService`, timezone id and timestamp.
2. Create a `TimezoneResult`, which is the output for info.
2. Call the `getTimezoneInfo` method from the `TimezoneService` using the timezoneId and timestamp from 1.) as input and `TimezoneResult` from 2) as output and `ProgressListener`.
3. Once the operation completes, the `TimezoneResult` from 2.) will be populated.
