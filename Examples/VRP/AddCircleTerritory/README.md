## Overview

This example app demonstrates the following features:
- Add a circle territory.

## How to use the sample

When you run the example app, a circle territory will be saved.

## How it works

1. Create a `vrp::Territory` and set the name, type (`Circle`), color and data. The data must contain the center's coordinates and the radius.
2. Create a `ProgressListener` and `vrp::Service`.
3. Call the `addTerritory()` method from the `vrp::Service` using the `vrp::Territory` and `ProgressListener` and wait for the operation to be done.
