// =============================================================================
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2014 projectchrono.org
// All right reserved.
//
// Use of this source code is governed by a BSD-style license that can be found
// in the LICENSE file at the top level of the distribution and at
// http://projectchrono.org/license-chrono.txt.
//
// =============================================================================
// Authors: Radu Serban, Asher Elmquist, Rainer Gericke
// =============================================================================
//
// Sample test program for sequential lmtv 2.5 ton simulation. It demonstrates
// chassis torsion due to random wheel excitation.
//
// The vehicle reference frame has Z up, X towards the front of the vehicle, and
// Y pointing to the left.
// All units SI.
//
// =============================================================================

#define USE_IRRLICHT

#include "chrono_vehicle/ChConfigVehicle.h"
#include "chrono_vehicle/ChVehicleModelData.h"
#include "chrono_vehicle/terrain/RigidTerrain.h"
#ifdef USE_IRRLICHT
#include "chrono_vehicle/driver/ChIrrGuiDriver.h"
#include "chrono_vehicle/wheeled_vehicle/utils/ChWheeledVehicleIrrApp.h"
#endif
#include "chrono_vehicle/driver/ChPathFollowerDriver.h"
#include "chrono/utils/ChUtilsInputOutput.h"

#include "chrono_models/vehicle/mtv/LMTV.h"

#include "chrono_thirdparty/filesystem/path.h"

#include <chrono>
#include <thread>
#include <math.h>

using namespace chrono;
using namespace chrono::vehicle;
using namespace chrono::vehicle::mtv;

// =============================================================================

// Initial vehicle location and orientation
ChVector<> initLoc(0, 0, 1.0);
ChQuaternion<> initRot(1, 0, 0, 0);

// Visualization type for vehicle parts (PRIMITIVES, MESH, or NONE)
VisualizationType chassis_vis_type = VisualizationType::MESH;
VisualizationType suspension_vis_type = VisualizationType::PRIMITIVES;
VisualizationType steering_vis_type = VisualizationType::PRIMITIVES;
VisualizationType wheel_vis_type = VisualizationType::MESH;
VisualizationType tire_vis_type = VisualizationType::MESH;

// Type of tire model (RIGID, TMEASY)
TireModelType tire_model = TireModelType::TMEASY;

// Point on chassis tracked by the camera
ChVector<> trackPoint(0.0, 0.0, 1.75);

ChVector<> vehCOM(-1.933, 0.014, 0.495);

// Simulation step sizes
double step_size = 1e-3;
double tire_step_size = step_size;

// Simulation end time
double tend = 60;

// Time interval between two render frames
double render_step_size = 1.0 / 50;  // FPS = 50
double output_step_size = 1e-2;

// vehicle driver inputs
// Desired vehicle speed (m/s)
double mph_to_ms = 0.44704;
double target_speed = 5 * mph_to_ms;

// output directory
const std::string out_dir = "../LMTV_QUALITY";
const std::string pov_dir = out_dir + "/POVRAY";
bool povray_output = false;
bool data_output = true;

std::string path_file("paths/straightOrigin.txt");
std::string steering_controller_file("mtv/SteeringController.json");
std::string speed_controller_file("mtv/SpeedController.json");

std::string rnd_1("terrain/meshes/uneven_300m_6m_10mm.obj");
std::string rnd_2("terrain/meshes/uneven_300m_6m_20mm.obj");
std::string rnd_3("terrain/meshes/uneven_300m_6m_30mm.obj");
std::string rnd_4("terrain/meshes/uneven_300m_6m_40mm.obj");

std::string output_file_name("quality");

std::string terrainFile = rnd_1;

// =============================================================================

int main(int argc, char* argv[]) {
    int terrainCode = 1;
    // read in argument as simulation duration
    switch (argc) {
        default:
        case 1:
            target_speed = 5;
            break;
        case 2:
            target_speed = atof(argv[1]);
            break;
        case 3:
            target_speed = atof(argv[1]);
            terrainCode = atoi(argv[2]);
            break;
    }
    switch (terrainCode) {
        case 1:
            terrainFile = rnd_1;
            break;
        case 2:
            terrainFile = rnd_2;
            break;
        case 3:
            terrainFile = rnd_3;
            break;
        case 4:
            terrainFile = rnd_4;
            break;
        default:
            std::cout << "Invalid Terrain Code - (1-4)";
    }

    output_file_name += "_" + std::to_string((int)target_speed);
    output_file_name += "_" + std::to_string((int)terrainCode);
    target_speed = target_speed * mph_to_ms;

    // --------------
    // Create systems
    // --------------

    // Create the vehicle, set parameters, and initialize
    LMTV lmtv;
    lmtv.SetContactMethod(ChContactMethod::NSC);
    lmtv.SetChassisFixed(false);
    lmtv.SetInitPosition(ChCoordsys<>(initLoc, initRot));
    lmtv.SetTireType(tire_model);
    lmtv.SetTireStepSize(tire_step_size);
    lmtv.Initialize();

    lmtv.SetChassisVisualizationType(chassis_vis_type);
    lmtv.SetSuspensionVisualizationType(suspension_vis_type);
    lmtv.SetSteeringVisualizationType(steering_vis_type);
    lmtv.SetWheelVisualizationType(wheel_vis_type);
    lmtv.SetTireVisualizationType(tire_vis_type);

    std::cout << "Vehicle mass:               " << lmtv.GetVehicle().GetVehicleMass() << std::endl;
    std::cout << "Vehicle mass (with tires):  " << lmtv.GetTotalMass() << std::endl;

    // ------------------
    // Create the terrain
    // ------------------
    double swept_radius = 0.01;

    auto patch_mat = chrono_types::make_shared<ChMaterialSurfaceNSC>();
    patch_mat->SetFriction(0.9f);
    patch_mat->SetRestitution(0.01f);
    RigidTerrain terrain(lmtv.GetSystem());
    auto patch = terrain.AddPatch(patch_mat, CSYSNORM, vehicle::GetDataFile(terrainFile), "test_mesh", swept_radius);
    patch->SetColor(ChColor(0.8f, 0.8f, 0.5f));
    patch->SetTexture(vehicle::GetDataFile("terrain/textures/dirt.jpg"), 12, 12);
    terrain.Initialize();

    // create the driver
    auto path = ChBezierCurve::read(vehicle::GetDataFile(path_file));
    ChPathFollowerDriver driver(lmtv.GetVehicle(), vehicle::GetDataFile(steering_controller_file),
                                vehicle::GetDataFile(speed_controller_file), path, "my_path", target_speed, false);
    driver.Initialize();

    // -------------------------------------
    // Create the vehicle Irrlicht interface
    // Create the driver system
    // -------------------------------------

#ifdef USE_IRRLICHT
    ChWheeledVehicleIrrApp app(&lmtv.GetVehicle(), L"LMTV ride & twist test");
    app.SetSkyBox();
    app.AddTypicalLights(irr::core::vector3df(30.f, -30.f, 100.f), irr::core::vector3df(30.f, 50.f, 100.f), 250, 130);
    app.SetChaseCamera(trackPoint, 6.0, 0.5);
    /*app.SetTimestep(step_size);*/
    app.AssetBindAll();
    app.AssetUpdateAll();

    // Visualization of controller points (sentinel & target)
    irr::scene::IMeshSceneNode* ballS = app.GetSceneManager()->addSphereSceneNode(0.1f);
    irr::scene::IMeshSceneNode* ballT = app.GetSceneManager()->addSphereSceneNode(0.1f);
    ballS->getMaterial(0).EmissiveColor = irr::video::SColor(0, 255, 0, 0);
    ballT->getMaterial(0).EmissiveColor = irr::video::SColor(0, 0, 255, 0);

#endif

    // -------------
    // Prepare output
    // -------------

    if (data_output || povray_output) {
        if (!filesystem::create_directory(filesystem::path(out_dir))) {
            std::cout << "Error creating directory " << out_dir << std::endl;
            return 1;
        }
    }
    if (povray_output) {
        if (!filesystem::create_directory(filesystem::path(pov_dir))) {
            std::cout << "Error creating directory " << pov_dir << std::endl;
            return 1;
        }
        driver.ExportPathPovray(out_dir);
    }

    utils::CSV_writer csv("\t");
    csv.stream().setf(std::ios::scientific | std::ios::showpos);
    csv.stream().precision(6);

    // Number of simulation steps between two 3D view render frames
    int render_steps = (int)std::ceil(render_step_size / step_size);

    // Number of simulation steps between two output frames
    int output_steps = (int)std::ceil(output_step_size / step_size);

    csv << "time";
    csv << "throttle";
    csv << "MotorSpeed";
    csv << "CurrentTransmissionGear";
    for (int i = 0; i < 4; i++) {
        csv << "WheelTorque";
    }
    for (int i = 0; i < 4; i++) {
        csv << "WheelAngVelX";
        csv << "WheelAngVelY";
        csv << "WheelAngVelZ";
    }
    csv << "VehicleSpeed";
    csv << "VehicleDriverAccelerationX";
    csv << "VehicleDriverAccelerationY";
    csv << "VehicleDriverAccelerationZ";

    csv << "VehicleCOMAccelerationX";
    csv << "VehicleCOMAccelerationY";
    csv << "VehicleCOMAccelerationZ";

    for (int i = 0; i < 4; i++) {
        csv << "TireForce";
    }
    csv << "EngineTorque";

    csv << "W0 Pos x"
        << "W0 Pos Y"
        << "W0 Pos Z"
        << "W1 Pos x"
        << "W1 Pos Y"
        << "W1 Pos Z";
    csv << "W2 Pos x"
        << "W2 Pos Y"
        << "W2 Pos Z"
        << "W3 Pos x"
        << "W3 Pos Y"
        << "W3 Pos Z";

    for (auto& axle : lmtv.GetVehicle().GetAxles()) {
        for (auto& wheel : axle->GetWheels()) {
            csv << wheel->GetPos();
        }
    }

    csv << std::endl;

    // ---------------
    // Simulation loop
    // ---------------

    std::cout << "data at: " << vehicle::GetDataFile(steering_controller_file) << std::endl;
    std::cout << "data at: " << vehicle::GetDataFile(speed_controller_file) << std::endl;
    std::cout << "data at: " << vehicle::GetDataFile(path_file) << std::endl;

    int step_number = 0;
    int render_frame = 0;

    double time = 0;

#ifdef USE_IRRLICHT
    while (app.GetDevice()->run() && (time < tend)) {
#else
    while (time < tend) {
#endif
        time = lmtv.GetSystem()->GetChTime();

#ifdef USE_IRRLICHT
        // path visualization
        const ChVector<>& pS = driver.GetSteeringController().GetSentinelLocation();
        const ChVector<>& pT = driver.GetSteeringController().GetTargetLocation();
        ballS->setPosition(irr::core::vector3df((irr::f32)pS.x(), (irr::f32)pS.y(), (irr::f32)pS.z()));
        ballT->setPosition(irr::core::vector3df((irr::f32)pT.x(), (irr::f32)pT.y(), (irr::f32)pT.z()));

        // std::cout<<"Target:\t"<<(irr::f32)pT.x()<<",\t "<<(irr::f32)pT.y()<<",\t "<<(irr::f32)pT.z()<<std::endl;
        // std::cout<<"Vehicle:\t"<<lmtv.GetVehicle().GetChassisBody()->GetPos().x()
        //   <<",\t "<<lmtv.GetVehicle().GetChassisBody()->GetPos().y()<<",\t "
        //   <<lmtv.GetVehicle().GetChassisBody()->GetPos().z()<<std::endl;

        // Render scene
        if (step_number % render_steps == 0) {
            app.BeginScene(true, true, irr::video::SColor(255, 140, 161, 192));
            app.DrawAll();
            app.EndScene();
        }

#endif

        if (povray_output && step_number % render_steps == 0) {
            char filename[100];
            sprintf(filename, "%s/data_%03d.dat", pov_dir.c_str(), render_frame + 1);
            utils::WriteShapesPovray(lmtv.GetSystem(), filename);
            render_frame++;
        }

        // Driver inputs
        ChDriver::Inputs driver_inputs = driver.GetInputs();

        // Update modules (process inputs from other modules)
        driver.Synchronize(time);
        terrain.Synchronize(time);
        lmtv.Synchronize(time, driver_inputs, terrain);

#ifdef USE_IRRLICHT
        app.Synchronize("Follower driver", driver_inputs);
#endif

        // Advance simulation for one timestep for all modules

        driver.Advance(step_size);
        terrain.Advance(step_size);
        lmtv.Advance(step_size);

#ifdef USE_IRRLICHT
        app.Advance(step_size);
#endif

        if (data_output && step_number % output_steps == 0) {
            // std::cout << time << std::endl;
            csv << time;
            csv << driver_inputs.m_throttle;
            csv << lmtv.GetVehicle().GetPowertrain()->GetMotorSpeed();
            csv << lmtv.GetVehicle().GetPowertrain()->GetCurrentTransmissionGear();
            for (int axle = 0; axle < 2; axle++) {
                csv << lmtv.GetVehicle().GetDriveline()->GetSpindleTorque(axle, LEFT);
                csv << lmtv.GetVehicle().GetDriveline()->GetSpindleTorque(axle, RIGHT);
            }
            for (int axle = 0; axle < 2; axle++) {
                csv << lmtv.GetVehicle().GetSpindleAngVel(axle, LEFT);
                csv << lmtv.GetVehicle().GetSpindleAngVel(axle, RIGHT);
            }
            csv << lmtv.GetVehicle().GetVehicleSpeed();
            csv << lmtv.GetVehicle().GetVehicleAcceleration(
                lmtv.GetVehicle().GetChassis()->GetLocalDriverCoordsys().pos);

            csv << lmtv.GetVehicle().GetVehicleAcceleration(vehCOM);

            for (auto& axle : lmtv.GetVehicle().GetAxles()) {
                for (auto& wheel : axle->GetWheels()) {
                    csv << wheel->GetTire()->ReportTireForce(&terrain).force;
                }
            }

            csv << lmtv.GetVehicle().GetPowertrain()->GetMotorTorque();

            csv << std::endl;
        }

        // Increment frame number
        step_number++;
    }

    if (data_output) {
        csv.write_to_file(out_dir + "/" + output_file_name + ".dat");
    }

    return 0;
}
