/* 
 * File:   ExtendedWorld.cpp
 * Author: pedro
 * 
 * Created on 17 de Fevereiro de 2014, 15:13
 */

#include "ExtendedWorld.h"

#include "interactions/VibrationSource.h"
#include "interactions/NotSimulated.h"
#include "interactions/WorldHeat.h"

using namespace Enki;

ExtendedWorld::ExtendedWorld (double width, double height, 
                              const Color& wallsColor, 
                              const World::GroundTexture& groundTexture):
	World (width, height, wallsColor, groundTexture),
	stateStream (NULL)
{
}

ExtendedWorld::ExtendedWorld (double r, const Color& wallsColor,
                              const World::GroundTexture& groundTexture):
	World (r, wallsColor, groundTexture),
	stateStream (NULL)
{
}

ExtendedWorld::ExtendedWorld ():
	World (),
	stateStream (NULL)
{
}

ExtendedWorld::~ExtendedWorld ()
{
	if (this->stateStream) {
		this->stateStream->close ();
		delete this->stateStream;
	}
}
void ExtendedWorld::addObject (PhysicalObject *o)
{
	World::addObject (o);
	ExtendedRobot *er = dynamic_cast<ExtendedRobot *> (o);
	if (er != NULL) {
		this->extendedRobots.insert (er);
	}
}

void ExtendedWorld::addPhysicSimulation (PhysicSimulation *pi)
{
	this->physicSimulations.push_back (pi);
	pi->initParameters (this);
}

void ExtendedWorld::saveStateTo (const char *fileName)
{
	if (this->stateStream) {
		this->stateStream->close ();
	}
	this->stateStream = new std::ofstream (fileName, std::ios_base::out | std::ios_base::trunc);
}

void ExtendedWorld::step (double dt, unsigned physicsOversampling)
{
	const double overSampledDt = dt / (double) physicsOversampling;
	for (unsigned po = 0; po < physicsOversampling; po++) {
		// init physics interactions
		for (PhysicSimulationsIterator pi = physicSimulations.begin (); pi != physicSimulations.end (); ++pi) {
			(*pi)->initStateComputing (overSampledDt);
			for (ExtendedRobotsIterator eri = extendedRobots.begin (); eri != extendedRobots.end (); ++eri) {
				(*eri)->initPhysicInteractions (overSampledDt, *pi);
				(*eri)->doPhysicInteractions (overSampledDt, *pi);
				(*eri)->finalizePhysicInteractions (overSampledDt, *pi);
			}
			(*pi)->computeNextState (overSampledDt);
		}
	}
	World::step (dt, physicsOversampling);
	if (this->stateStream) {
		// *(this->stateStream) << dt << '\n';
		for (PhysicSimulationsIterator pi = physicSimulations.begin (); pi != physicSimulations.end (); ++pi) {
			WorldHeat *worldHeat = dynamic_cast<WorldHeat *> (*pi);
			if (worldHeat) {
				worldHeat->dumpState (*(this->stateStream));
			}
		}
		// *(this->stateStream) << '\n';
	}
}

double ExtendedWorld::getVibrationAmplitudeAt (const Point &position, double time) const
{
	double result = 0;
	for (ObjectsIterator i = this->objects.begin (); i != this->objects.end (); ++i) {
		PhysicalObject *po = (*i);
		VibrationSource *vibrationSource = dynamic_cast<VibrationSource *> (po);
		if (vibrationSource != NULL) {
			try {
				result += vibrationSource->getAmplitudeAt (position, time);
			}
			catch (NotSimulated *ns) {
			}
		}
	}
	return result;
}

double ExtendedWorld::getVibrationIntensityAt (const Point &position) const
{
	double result = 0;
	for (ObjectsIterator i = this->objects.begin (); i != this->objects.end (); ++i) {
		PhysicalObject *po = (*i);
		VibrationSource *vibrationSource = dynamic_cast<VibrationSource *> (po);
		if (vibrationSource != NULL) {
			try {
				result += vibrationSource->getIntensityAt (position);
			}
			catch (NotSimulated *ns) {
			}
		}
	}
	return result;
}
