## Overview

This example app demonstrates the following features:
- Get all the optimizations created.

## How to use the sample

When you run the example app, all the optimizations will be returned.

## How it works

1. Create a `ProgressListener`, a `vrp::Service` and a `vrp::OptimizationList`.
2. Call the `getAllOptimizations()` method from the `vrp::Service` using the list from 1.) and the `ProgressListener`.
3. Once the operation completes, the list from 1.) will be populated.
