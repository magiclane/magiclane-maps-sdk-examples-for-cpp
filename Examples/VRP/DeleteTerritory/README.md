## Overview

This example app demonstrates the following features:
- Delete a territory.

## How to use the sample

When you run the example app, a territory will be deleted.

## How it works

1. Create a `ProgressListener` and `vrp::Service`.
2. Call the `deleteTerritory()` method from the `vrp::Service` using the territory's ID and `ProgressListener` and wait for the operation to be done.
