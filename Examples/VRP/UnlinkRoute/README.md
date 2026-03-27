## Overview

This example app demonstrates the following features:
- Unlink a route from its optimization.

The route will no longer be a part of the optimization's solution. The orders visited in this route will be deleted from the optimization.
A new optimization will be created for the unlinked route. The new optimization will have same configuration parameters, vehicle constraints and the rest of the fields as the unlinked route.

The route cannot be unlinked if it is the only route in the optimization's solution.

## How to use the sample

When you run the example app, the route will be unlinked.

## How it works

1. Create a `ProgressListener` and a `vrp::Service`.
2. Call the `unlinkRoute()` method from the `vrp::Service` using the route's id which will be unlinked and the `ProgressListener` and wait for the operation to be done.

**Note:** The route's ID won't change so it can be retrieved using the same `routeId`.
