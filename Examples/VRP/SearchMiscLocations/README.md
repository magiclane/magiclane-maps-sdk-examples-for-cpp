## Overview

This example app demonstrates the following features:
- Retrieve miscellaneous locations from the database that match the specified search filter.

## How to use the sample

When you run the example app, all miscellaneous locations from the database that contain the `searchTerm` will be retrieved. The `searchTerm` is a string used to search for locations, and it is matched against each field in the miscellaneous location object.

## How it works

1. Initialize `vrp::MiscLocation` objects with desired coordinates and address details.
2. Create a `ProgressListener`, `vrp::Service`, and a `vrp::MiscLocationList`.
3. Add the initialized miscellaneous locations to the database by calling the `addMiscLocation()` method from the `vrp::Service`, and wait for the operation to complete.
4. After adding the locations, call the `getMiscLocations()` method from the `vrp::Service` using the list from step 2.) and the `ProgressListener`.
5. Once the operation completes, the list from step 2.) will be populated with miscellaneous locations that match the search criteria.
