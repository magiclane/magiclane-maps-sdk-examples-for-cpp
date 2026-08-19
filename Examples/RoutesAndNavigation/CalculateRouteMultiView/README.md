## Overview

This example app demonstrates the following features:
- Calculate routes and display them in separate views

![](screenshot.png)

## How to use the sample

When you run the example app, you'll be viewing the scene from above. The screen will be split into four identical views (quadrants). From each view a fly will be performed to a different calculated route.

## How it works

1. Create a `MapServiceListener`, one `OpenGLContext`, one `Screen` and four `MapView` objects, one per screen quadrant (`gem::RectF`).
2. Create a `RouteList`, a `LandmarkList` with two Landmarks in it and a `RoutePreferences` object.
3. Call the `RoutingService` using `RouteList`, `LandmarkList`, `RoutePreferences` and the progress listener.
4. Perform steps 2 and 3 for each of the other `MapView` objects, with different waypoints.
5. Once the route calculation operations complete, add the first calculated route of each collection to their corresponding view `MapViewPreferences` routes collection.
6. Instruct each `MapView` object to center on its route.
