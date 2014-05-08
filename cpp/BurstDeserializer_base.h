/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This file is part of REDHAWK Basic Components BurstDeserializer.
 *
 * REDHAWK Basic Components BurstDeserializer is free software: you can redistribute it and/or modify it under the terms of
 * the GNU Lesser General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * REDHAWK Basic Components BurstDeserializer is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with this
 * program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef BURSTDESERIALIZER_IMPL_BASE_H
#define BURSTDESERIALIZER_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Resource_impl.h>
#include <ossie/ThreadedComponent.h>

#include <bulkio/bulkio.h>

class BurstDeserializer_base : public Resource_impl, protected ThreadedComponent
{
    public:
        BurstDeserializer_base(const char *uuid, const char *label);
        ~BurstDeserializer_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        bool transpose;

        // Ports
        bulkio::InDoublePort *input;
        bulkio::OutDoublePort *output;

    private:
};
#endif // BURSTDESERIALIZER_IMPL_BASE_H
