// =============================================================================
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2014 projectchrono.org
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file at the top level of the distribution and at
// http://projectchrono.org/license-chrono.txt.
//
// =============================================================================
// Authors: Radu Serban, Justin Madsen
// =============================================================================
//
// Simple powertrain model for the HMMWV vehicle.
// - trivial speed-torque curve
// - no torque converter
// - no transmission box
//
// =============================================================================

#include "chrono_models/vehicle/mtv/LMTV_SimplePowertrain.h"

namespace chrono {
namespace vehicle {
namespace mtv {

// -----------------------------------------------------------------------------
// Static variables
// -----------------------------------------------------------------------------
const double LMTV_SimplePowertrain::m_max_torque = 2400 / 8.851;
const double LMTV_SimplePowertrain::m_max_speed = 2000;
const double LMTV_SimplePowertrain::m_fwd_gear_ratio = 0.3;
const double LMTV_SimplePowertrain::m_rev_gear_ratio = -0.3;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
LMTV_SimplePowertrain::LMTV_SimplePowertrain(const std::string& name) : ChSimplePowertrain(name) {}

}  // namespace mtv
}  // end namespace vehicle
}  // end namespace chrono
