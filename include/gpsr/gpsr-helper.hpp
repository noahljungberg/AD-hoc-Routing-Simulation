#ifndef GPSR_HELPER_H
#define GPSR_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-routing-helper.h"

namespace ns3 {

    /**
     * \brief Helper class that adds GPSR routing to nodes
     */
    class GpsrHelper : public Ipv4RoutingHelper
    {
    public:
        /**
         * \brief Constructor
         */
        GpsrHelper();

        /**
         * \brief Copy constructor
         * \return Pointer to cloned helper
         */
        GpsrHelper* Copy() const;

        /**
         * \brief Create a routing protocol and install it on a node
         * \param node Node to install the routing protocol on
         * \return Pointer to the newly created routing protocol
         */
        virtual Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const;

        /**
         * \brief Set an attribute for the routing protocol
         * \param name Name of the attribute
         * \param value Value of the attribute
         */
        void Set(std::string name, const AttributeValue &value);

        /**
         * \brief Install GPSR on all nodes in a container
         */
        void Install() const;

    private:
        ObjectFactory m_agentFactory; //!< Object factory to create GPSR instances
    };

} // namespace ns3

#endif // GPSR_HELPER_H