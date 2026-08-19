## Overview

This example app demonstrates the following features:
- Delete an optimization.

## How to use the sample

When you run the example app, an optimization will be deleted.

## How it works

1. Create a `ProgressListener` and `vrp::Service`.
2. Retrieve the optimization to delete with `getOptimization()`, using your own optimization id (the sample has a `-1` placeholder that must be replaced).
3. Call the `deleteOptimization()` method from the `vrp::Service` using a list of optimization ids (`{ optimization.getId() }`) and the `ProgressListener`, and wait for the operation to be done.
