%{

/* Includes the header in the wrapper code */
#include "chrono/physics/ChContactContainerBase.h"

using namespace collision;


// NESTED CLASSES: inherit stubs (not virtual) as outside classes

class ChReportContactCallbackP : public chrono::ChContactContainerBase::ReportContactCallback {
    public:
        ChReportContactCallbackP() {}
        virtual bool OnReportContact(const chrono::ChVector<>& pA,
                                     const chrono::ChVector<>& pB,
                                     const chrono::ChMatrix33<>& plane_coord,
                                     const double& distance,
                                     const chrono::ChVector<>& react_forces,
                                     const chrono::ChVector<>& react_torques,
                                     chrono::ChContactable* contactobjA,
                                     chrono::ChContactable* contactobjB) {
            GetLog() << "You must implement OnReportContact()!\n";
            return false;
        }
};

class ChAddContactCallbackP : public chrono::ChContactContainerBase::AddContactCallback {
    public:
        ChAddContactCallbackP() {}
        virtual void OnAddContact(const chrono::collision::ChCollisionInfo& contactinfo,
                                  chrono::ChMaterialComposite* const material) {
            GetLog() << "You must implement OnAddContact()!\n";
        }
};

%}


// Forward ref
%import "ChCollisionModel.i"
%import "ChCollisionInfo.i"


// Cross-inheritance between Python and c++ for callbacks that must be inherited.
// Put this 'director' feature _before_ class wrapping declaration.

%feature("director") ChReportContactCallbackP;
%feature("director") ChAddContactCallbackP;


// NESTED CLASSES

class ChReportContactCallbackP {
  public:
    virtual bool OnReportContact(const chrono::ChVector<>& pA,
                                 const chrono::ChVector<>& pB,
                                 const chrono::ChMatrix33<>& plane_coord,
                                 const double& distance,
                                 const chrono::ChVector<>& react_forces,
                                 const chrono::ChVector<>& react_torques,
                                 chrono::ChContactable* contactobjA,
                                 chrono::ChContactable* contactobjB) {return false;}	
};

class ChAddContactCallbackP {
  public:
	virtual void OnAddContact(const chrono::collision::ChCollisionInfo& contactinfo,
                              chrono::ChMaterialComposite* const material) {}
};

%extend chrono::ChContactContainerBase
{
	void RegisterAddContactCallback(::ChAddContactCallbackP* callback) {
	    $self->RegisterAddContactCallback(callback);
	}
};

%ignore chrono::ChContactContainerBase::RegisterAddContactCallback();


/* Parse the header file to generate wrappers */
%include "../chrono/physics/ChContactContainerBase.h"    

