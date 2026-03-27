## Overview

This example app demonstrates the following features:
- Delete an optimization.

## How to use the sample

When you run the example app, an optimization will be deleted.

## How it works

1. Create a `ProgressListener` and `vrp::Service`.
2. Call the `deleteOptimization()` method from the `vrp::Service` using the optimization's id and `ProgressListener` and wait for the operation to be done.
