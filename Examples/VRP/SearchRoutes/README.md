## Overview

This example app demonstrates the following features:
- Retrieve routes from the database that match the specified search filter.

## How to use the sample

When you run the example app, all routes in the database that contain the `searchTerm` will be retrieved. The `searchTerm` is a string used to search for routes, and it is matched against the fields in the routes object.

## How it works

1. Create a `vrp::OrderList` and add the orders to it. Each order needs to have a customer set; you can either add a new customer and then set it to the order, or you cand use a previously created customer (see [Get Customer](../GetCustomer) example).
2. Create a `vrp::ConfigurationParameters` and set the desired parameters.
3. Create one `vrp::VehicleConstraints` for each vehicle and set them to a `vrp::VehicleConstraintsList`.
4. Create a `vrp::Optimization`, set the desired fields and the objects created at 1.), 2.) and 3.) to it.
5. Create a `ProgressListener`, `vrp::Service`, a `vrp::Request` to track the operation, and a `vrp::RouteList` for both the solution and the search results.
6. Call the `addOptimization()` method from `vrp::Service` using the `vrp::Optimization` from 4.), the `vrp::Request` from 5.) and the progress listener. This sample repeats steps 1.) to 6.) for a second optimization, so that the search below has more than one candidate to match against: they are named "Paris intra-muros optimization1" and "Paris intra-muros optimization2", so that the filter matches the routes of the first one only.
7. Monitor each request until it reaches a finished state, then call `getSolution()` on the corresponding `vrp::Optimization` to obtain its `vrp::RouteList`.
8. Call the `getRoutes()` method from the `vrp::Service` using a `vrp::RouteList`, the `ProgressListener` and the text to search for (`"optimization1"` here).
9. Once the operation completes, that list will be populated with the routes that match the search criteria.
