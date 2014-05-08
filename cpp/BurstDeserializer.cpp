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
 *//**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "BurstDeserializer.h"

PREPARE_LOGGING(BurstDeserializer_i)

BurstDeserializer_i::BurstDeserializer_i(const char *uuid, const char *label) :
    BurstDeserializer_base(uuid, label),
    flushStreams(false)
{
	addPropertyChangeListener("transpose", this, &BurstDeserializer_i::transposeChanged);
}

BurstDeserializer_i::~BurstDeserializer_i()
{
}

/***********************************************************************************************

    Basic functionality:

        The service function is called by the serviceThread object (of type ProcessThread).
        This call happens immediately after the previous call if the return value for
        the previous call was NORMAL.
        If the return value for the previous call was NOOP, then the serviceThread waits
        an amount of time defined in the serviceThread's constructor.
        
    SRI:
        To create a StreamSRI object, use the following code:
                std::string stream_id = "testStream";
                BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);

	Time:
	    To create a PrecisionUTCTime object, use the following code:
                BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();

        
    Ports:

        Data is passed to the serviceFunction through the getPacket call (BULKIO only).
        The dataTransfer class is a port-specific class, so each port implementing the
        BULKIO interface will have its own type-specific dataTransfer.

        The argument to the getPacket function is a floating point number that specifies
        the time to wait in seconds. A zero value is non-blocking. A negative value
        is blocking.  Constants have been defined for these values, bulkio::Const::BLOCKING and
        bulkio::Const::NON_BLOCKING.

        Each received dataTransfer is owned by serviceFunction and *MUST* be
        explicitly deallocated.

        To send data using a BULKIO interface, a convenience interface has been added 
        that takes a std::vector as the data input

        NOTE: If you have a BULKIO dataSDDS port, you must manually call 
              "port->updateStats()" to update the port statistics when appropriate.

        Example:
            // this example assumes that the component has two ports:
            //  A provides (input) port of type bulkio::InShortPort called short_in
            //  A uses (output) port of type bulkio::OutFloatPort called float_out
            // The mapping between the port and the class is found
            // in the component base class header file

            bulkio::InShortPort::dataTransfer *tmp = short_in->getPacket(bulkio::Const::BLOCKING);
            if (not tmp) { // No data is available
                return NOOP;
            }

            std::vector<float> outputData;
            outputData.resize(tmp->dataBuffer.size());
            for (unsigned int i=0; i<tmp->dataBuffer.size(); i++) {
                outputData[i] = (float)tmp->dataBuffer[i];
            }

            // NOTE: You must make at least one valid pushSRI call
            if (tmp->sriChanged) {
                float_out->pushSRI(tmp->SRI);
            }
            float_out->pushPacket(outputData, tmp->T, tmp->EOS, tmp->streamID);

            delete tmp; // IMPORTANT: MUST RELEASE THE RECEIVED DATA BLOCK
            return NORMAL;

        If working with complex data (i.e., the "mode" on the SRI is set to
        true), the std::vector passed from/to BulkIO can be typecast to/from
        std::vector< std::complex<dataType> >.  For example, for short data:

            bulkio::InShortPort::dataTransfer *tmp = myInput->getPacket(bulkio::Const::BLOCKING);
            std::vector<std::complex<short> >* intermediate = (std::vector<std::complex<short> >*) &(tmp->dataBuffer);
            // do work here
            std::vector<short>* output = (std::vector<short>*) intermediate;
            myOutput->pushPacket(*output, tmp->T, tmp->EOS, tmp->streamID);

        Interactions with non-BULKIO ports are left up to the component developer's discretion

    Properties:
        
        Properties are accessed directly as member variables. For example, if the
        property name is "baudRate", it may be accessed within member functions as
        "baudRate". Unnamed properties are given a generated name of the form
        "prop_n", where "n" is the ordinal number of the property in the PRF file.
        Property types are mapped to the nearest C++ type, (e.g. "string" becomes
        "std::string"). All generated properties are declared in the base class
        (BurstDeserializer_base).
    
        Simple sequence properties are mapped to "std::vector" of the simple type.
        Struct properties, if used, are mapped to C++ structs defined in the
        generated file "struct_props.h". Field names are taken from the name in
        the properties file; if no name is given, a generated name of the form
        "field_n" is used, where "n" is the ordinal number of the field.
        
        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            //  - A boolean called scaleInput
              
            if (scaleInput) {
                dataOut[i] = dataIn[i] * scaleValue;
            } else {
                dataOut[i] = dataIn[i];
            }
            
        Callback methods can be associated with a property so that the methods are
        called each time the property value changes.  This is done by calling 
        addPropertyChangeListener(<property name>, this, &BurstDeserializer_i::<callback method>)
        in the constructor.

        Callback methods should take two arguments, both const pointers to the value
        type (e.g., "const float *"), and return void.

        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            
        //Add to BurstDeserializer.cpp
        BurstDeserializer_i::BurstDeserializer_i(const char *uuid, const char *label) :
            BurstDeserializer_base(uuid, label)
        {
            addPropertyChangeListener("scaleValue", this, &BurstDeserializer_i::scaleChanged);
        }

        void BurstDeserializer_i::scaleChanged(const float *oldValue, const float *newValue)
        {
            std::cout << "scaleValue changed from" << *oldValue << " to " << *newValue
                      << std::endl;
        }
            
        //Add to BurstDeserializer.h
        void scaleChanged(const float* oldValue, const float* newValue);
        

void BurstDeserializer_i::updateSriTranspose(unsigned int complex_offset, bulkio::InDoublePort::dataTransfer* tmp)
{
		size_t nstreams = tmp->dataBuffer.size() / (tmp->SRI.subsize * complex_offset);
		for (unsigned int i = 0; i < nstreams; i++,streamCount++) {
			std::ostringstream newstreamid;
			newstreamid << tmp->streamID << "_" << streamCount;
			BULKIO::StreamSRI newsri = tmp->SRI;
			newsri.streamID = newstreamid.str().c_str();
			this->output->pushSRI(newsri);
		}
}
        
************************************************************************************************/
int BurstDeserializer_i::serviceFunction()
{
    bulkio::InDoublePort::dataTransfer *tmp = this->input->getPacket(-1);
    if (tmp == NULL)
    	return NOOP;

    bool thisTranspose=transpose;

	if (tmp->inputQueueFlushed) {
		LOG_WARN(BurstDeserializer_i, "input queue flushed - data has been thrown on the floor.");
		flushStreams=true;
	}

	if (flushStreams) {
		LOG_DEBUG(BurstDeserializer_i, "flushing streams");
		std::vector<double> data;
		//clear any active streams and push EOS for each stream
		for (state_type::iterator i = activeStreams.begin(); i!=activeStreams.end(); i++) {
			for (std::vector<std::string>::iterator outID = i->second.outputIDs.begin(); outID !=i->second.outputIDs.end(); outID++) {
				this->output->pushPacket(data,tmp->T,true,*outID);
			}
		}
		activeStreams.clear();
		flushStreams=false;
	}

    //ensure this is run at least once the very first time
	state_type::iterator state = activeStreams.find(tmp->streamID);
	if (state == activeStreams.end())
	{
		LOG_DEBUG(BurstDeserializer_i, "New input stream:  " << tmp->streamID);
		state_type::value_type vt(tmp->streamID, StateStruct());
		state = activeStreams.insert(activeStreams.end(),vt);
		state->second.streamCount=0;
		state->second.adjustXStart = (tmp->SRI.xunits==BULKIO::UNITS_TIME && tmp->SRI.yunits==BULKIO::UNITS_TIME);
	}

	//double check to ensure subsize changes are checked explicitly
	bool subsizeRefresh = thisTranspose && (tmp->SRI.subsize != state->second.outputIDs.size());

	//update state if we are brand new or things have chagned
	if (tmp->sriChanged || subsizeRefresh || state->second.streamCount==0) {
		updateState(subsizeRefresh, state->second, thisTranspose, tmp);
	}

	//now we will do sri pushes and output data
	if (tmp->SRI.subsize > 0) {
		//typical case - we have a valid subsize
	    unsigned int complex_offset;
	    if (tmp->SRI.mode == 0) {
	    	complex_offset = 1;
	    } else {
	    	complex_offset = 2;
	    }
	    //number of elements (including two floating point numbers per element if data is complex)
	    size_t numElements = tmp->dataBuffer.size()/complex_offset;

	    if (numElements % tmp->SRI.subsize !=0)
	    	LOG_WARN(BurstDeserializer_i, "numElements "<<numElements <<" and subsize "<< tmp->SRI.subsize<<" does not yield an integer multiple of frames.  Something wierd is going on with the data packet size");

		if (state->second.adjustXStart && (tmp->SRI.xstart!=0 || tmp->SRI.ystart!=0) && (!thisTranspose || tmp->sriChanged)) {
		    //adjust xstart if required prior to any sri pushes
			if (tmp->SRI.xstart !=0) {
				state->second.SRI.xstart=tmp->SRI.xstart;
				if ((tmp->SRI.ystart!=0) && (tmp->SRI.ystart!= tmp->SRI.xstart))
					LOG_WARN(BurstDeserializer_i, "xstart & ystart values differ for time vs time raster.  Using xstart value");
			} else
				state->second.SRI.xstart=tmp->SRI.ystart;
		}

		if (thisTranspose)
			pushTransposed(numElements, complex_offset, state, tmp);
		else
			pushUnTransposed(complex_offset, state, tmp);

	} else {
		if (tmp->sriChanged)
			this->output->pushSRI(tmp->SRI);
		this->output->pushPacket(tmp->dataBuffer,tmp->T,tmp->EOS,tmp->streamID);
	}
	delete tmp;

    return NORMAL;
}

void BurstDeserializer_i::transposeChanged(const bool *oldValue, const bool *newValue)
{
	LOG_DEBUG(BurstDeserializer_i, "transposed changed from "<< *oldValue << " to " << *newValue);
	flushStreams = (*oldValue!=*newValue);
}

template<typename T>
void BurstDeserializer_i::demuxData(std::vector<double>& input, std::vector<T>& output, size_t colNum, size_t subsize)
{
	//take every n elements starting with columnNum and copy them to output
	std::vector<T>* inVec = (std::vector<T>*) &(input);
	size_t outSize =inVec->size()/subsize;
	output.clear();
	output.reserve(outSize);

	for (typename std::vector<T>::iterator i = inVec->begin()+colNum; i < inVec->end(); i+=subsize)
		output.push_back(*i);
}

std::string BurstDeserializer_i::getStreamID(state_type::iterator state)
{
	//get the next streamID for this state
	std::ostringstream newstreamid;
	newstreamid<<state->first<<"_"<<state->second.streamCount;
	state->second.streamCount++;
	return newstreamid.str();
}

void BurstDeserializer_i::updateState(bool subsizeRefresh,  StateStruct& state, bool thisTranspose, bulkio::InDoublePort::dataTransfer* tmp) {
	LOG_DEBUG(BurstDeserializer_i, "updating SRI for stream:  " << tmp->streamID);
	if (tmp->SRI.subsize >0 ){
		//force an sri push later
		tmp->sriChanged = true;
		//if subsize has changed clear all streams and start over
		if (subsizeRefresh) {
			if (!state.outputIDs.empty()) {
				std::vector<double> data;
				LOG_DEBUG(BurstDeserializer_i,
						"clearing out old streams due to subsize change");
				for (std::vector<std::string>::iterator outID = state.outputIDs.begin(); outID != state.outputIDs.end(); outID++) {
					this->output->pushPacket(data, tmp->T, true, *outID);
				}
				state.outputIDs.clear();
			}
			state.streamCount = 0;
		}
		//now update all the state SRI accordingly
		state.SRI = tmp->SRI;
		//now update our SRI to be what we need for the output values
		if (thisTranspose) {
			if (tmp->SRI.ydelta > 0)
				state.SRI.xdelta = tmp->SRI.ydelta;
			else {
				state.SRI.xdelta = tmp->SRI.xdelta / tmp->SRI.subsize;
				LOG_WARN(BurstDeserializer_i, "ydelta "<<tmp->SRI.ydelta <<" is invalid.  Using best guess "<<state.SRI.xdelta);
			}
			state.SRI.xstart = tmp->SRI.ystart;
			state.SRI.xunits = tmp->SRI.yunits;
			//if both units are time then we will need to adjust the xstart for each particular stream
		}
		//reset all the subsize information
		state.SRI.subsize = 0;
		state.SRI.ystart = 0;
		state.SRI.ydelta = 0;
		state.SRI.yunits = BULKIO::UNITS_NONE;
	}
	else
		LOG_WARN(BurstDeserializer_i, "burst deserializer found stream "<< tmp->SRI.streamID<<" with subsize "<<tmp->SRI.subsize<<".  Treating as pass threw");

}

void BurstDeserializer_i::pushTransposed(size_t numElements, unsigned int complex_offset,
		state_type::iterator state, bulkio::InDoublePort::dataTransfer* tmp) {
	// we are not transposing the data
	std::vector<double> data;
	data.reserve(numElements * complex_offset);
	std::vector<std::complex<double> >* dataCx = (std::vector<std::complex<double> >*) (&(data));
	bool sriPush;
	for (size_t colNum = 0; colNum != tmp->SRI.subsize; colNum++) {
		if ((tmp->SRI.mode == 0))
			demuxData(tmp->dataBuffer, data, colNum, tmp->SRI.subsize);
		else
			demuxData(tmp->dataBuffer, *dataCx, colNum, tmp->SRI.subsize);

		sriPush = tmp->sriChanged;
		//make sure we have enough active streamIDs.  If not start a new one here and force a sri push
		if (state->second.outputIDs.size() == colNum) {
			state->second.outputIDs.push_back(getStreamID(state));
			sriPush = true;
		}
		if (sriPush) {
			//push sri if we need to do so
			state->second.SRI.streamID = state->second.outputIDs[colNum].c_str();
			this->output->pushSRI(state->second.SRI);
			if (state->second.adjustXStart)
				state->second.SRI.xstart += tmp->SRI.xdelta;
		}
		this->output->pushPacket(data, tmp->T, tmp->EOS,
				state->second.outputIDs[colNum]);
	}
}

void BurstDeserializer_i::pushUnTransposed(unsigned int complex_offset,
		state_type::iterator state,
		bulkio::InDoublePort::dataTransfer* tmp) {
	// we are transposing the data
	std::vector<double> data;
	size_t stride = tmp->SRI.subsize * complex_offset;
	unsigned int nstreams = tmp->dataBuffer.size()/stride;
	data.reserve(stride);
	for (unsigned int i = 0; i!=nstreams; i++)
	{
		data.assign(tmp->dataBuffer.begin() + (i * stride), tmp->dataBuffer.begin() + ((i + 1) * stride));
		//we always create a new stream for each push and send eos for each new stream
		std::string streamID = getStreamID(state);
		state->second.SRI.streamID = streamID.c_str();
		this->output->pushSRI(state->second.SRI);
		this->output->pushPacket(data, tmp->T, true, streamID);
		if (state->second.adjustXStart)
			state->second.SRI.xstart += tmp->SRI.ydelta;
	}
}

