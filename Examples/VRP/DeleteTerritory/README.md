## Overview

This example app demonstrates the following features:
- Delete a territory.

## How to use the sample

When you run the example app, a territory will be deleted.

## How it works

1. Create a `ProgressListener` and `vrp::Service`.
2. Obtain the territory to delete. This sample creates a circle `vrp::Territory` and adds it with `addTerritory()` first; you can equally retrieve an existing one (see [Get Territory](../GetTerritory) example).
3. Call the `deleteTerritory()` method from the `vrp::Service` using a list of territory ids (`{ territory.getId() }`) and the `ProgressListener`, and wait for the operation to be done.
