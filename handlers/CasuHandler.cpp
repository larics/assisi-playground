/*

 */
#include <iostream>

#include <boost/foreach.hpp>

#include <zmq.hpp>
#include "playground/zmq_helpers.hpp"

#include "robots/Casu.h"
#include "handlers/CasuHandler.h"

// Protobuf message headers
#include "base_msgs.pb.h"
#include "dev_msgs.pb.h"
#include "sim_msgs.pb.h"

using zmq::message_t;
using zmq::socket_t;
using std::string;
using std::cerr;
using std::endl;

using namespace AssisiMsg;

namespace Enki
{

    /* virtual */
    string CasuHandler::createObject(const std::string& data, 
                                     World* world)
    {
        string name = "";
        Spawn spawn_msg;     
        assert(spawn_msg.ParseFromString(data));
        if (casus_.count(spawn_msg.name()) < 1)
        {
            name = spawn_msg.name();
            Point pos(spawn_msg.pose().position().x(),
                      spawn_msg.pose().position().y());
            double yaw(spawn_msg.pose().orientation().z());
            casus_[name] = new Casu(world);
            casus_[name]->pos = pos;
            casus_[name]->angle = yaw;
            world->addObject(casus_[name]);
        }
        else
        {
            cerr << "Casu "<< spawn_msg.name() << " already exists." << endl;
        }
        return name;
    }

// -----------------------------------------------------------------------------

    /* virtual */
    int CasuHandler::handleIncoming(const std::string& name,
                                    const std::string& device,
                                    const std::string& command,
                                    const std::string& data)
    {
        int count = 0;
        if (device == "DiagnosticLed")
        {
            if (command == "On")
            {
                ColorStamped color_msg;
                assert(color_msg.ParseFromString(data));
                casus_[name]->top_led->on( Enki::Color(color_msg.color().red(),
                                                      color_msg.color().green(),
                                                      color_msg.color().blue(),
                                                      color_msg.color().alpha() ) );
                count++;
            }
            else if (command == "Off")
            {
                casus_[name]->top_led->off( );
                count++;
            }
            else
            {
                cerr << "Unknown command for " << name << "/" << device << endl;
                return 0;
            }         
        }
        else
        {
            cerr << "Unknown device " << device << endl;
        }
        return count;
    }

// -----------------------------------------------------------------------------

    /* virtual */
    int CasuHandler::sendOutgoing(socket_t& socket)
    {
        int count = 0;
        BOOST_FOREACH(const CasuMap::value_type& ca, casus_)
        {
            std::string data;
            /* Publishing IR readings */
            RangeArray ranges;
            BOOST_FOREACH(IRSensor* ir, ca.second->range_sensors)
            {
                ranges.add_range(ir->getDist());                
            }
            ranges.SerializeToString(&data);
            zmq::send_multipart(socket, ca.first, "ir", "ranges", data);

            /* Publish other stuff as necessary ... */

            count++;
        }
        return count;
    }
// -----------------------------------------------------------------------------

}
