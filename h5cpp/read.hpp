/*
 * Copyright (c) 2018 vargaconsulting, Toronto,ON Canada
 * Author: Varga, Steven <steven@vargaconsulting.ca>
 */

#include <hdf5.h>
#include "macros.h"
#include <initializer_list>
#include <tuple>
#include <type_traits>

#ifndef  H5CPP_READ_H 
#define  H5CPP_READ_H

namespace h5 {
/***************************  REFERENCE *****************************/

 	/** \func_read_hdr
 	*  Updates the content of passed **ptr** pointer, which must have enough memory space to receive data.
	*  Optional arguments **args:= h5::offset | h5::stride | h5::count | h5::block** may be specified for partial IO, 
	*  to describe the retrieved hyperslab from hdf5 file space. Default case is to select and retrieve all elements from dataset. 
	*  **h5::dxpl_t** provides control to datatransfer. 
	* \code
	* h5::fd_t fd = h5::open("myfile.h5", H5F_ACC_RDWR);
	* h5::ds_t ds = h5::open(fd,"path/to/dataset");
	* std::vector<float> myvec(10*10);
	* auto err = h5::read( fd, "path/to/dataset", myvec.data(), h5::count{10,10}, h5::offset{5,0} );	
	* \endcode  
	* \par_file_path \par_dataset_path \par_ptr \par_offset \par_stride \par_count \par_block \par_dxpl  \tpar_T \returns_err
 	*/ 

	template<class T, class... args_t>
	std::enable_if_t<!std::is_same<T,char*>::value,
	h5::herr_t> read( const h5::ds_t& ds, T* ptr, args_t&&... args ){
		using toffset  = typename arg::tpos<const h5::offset_t&,const args_t&...>;
		using tstride  = typename arg::tpos<const h5::stride_t&,const args_t&...>;
		using tcount   = typename arg::tpos<const h5::count_t&,const args_t&...>;
		using tblock   = typename arg::tpos<const h5::block_t&,const args_t&...>;

		static_assert( tcount::present, "h5::count_t{ ... } must be specified" );
		static_assert( utils::is_supported<T>, "error: " H5CPP_supported_elementary_types );

		auto tuple = std::forward_as_tuple(args...);
		const h5::count_t& count = std::get<tcount::value>( tuple );

		h5::offset_t  default_offset{0,0,0,0,0,0};
		const h5::offset_t& offset = arg::get( default_offset, args...);

		h5::stride_t  default_stride{1,1,1,1,1,1,1};
		const h5::stride_t& stride = arg::get( default_stride, args...);

		h5::block_t  default_block{1,1,1,1,1,1,1};
		const h5::block_t& block = arg::get( default_block, args...);

		h5::count_t size; // compute actual memory space
		for(int i=0;i<count.rank;i++) size[i] = count[i] * block[i];
		size.rank = count.rank;

		const h5::dxpl_t& dxpl = arg::get( h5::default_dxpl, args...);

		hid_t file_space = H5Dget_space(ds);
        hsize_t rank = H5Sget_simple_extent_ndims(file_space);
		//TODO: error handler
		if( rank != count.rank ) return -1;

		hid_t mem_type = utils::h5type<T>();
		hid_t mem_space  = H5Screate_simple(rank, *size, NULL);
		H5Sselect_all(mem_space);
		H5Sselect_hyperslab(file_space, H5S_SELECT_SET, *offset, *stride, *count, *block);

		H5Dread(ds, mem_type, mem_space, file_space, dxpl, ptr );
		H5Tclose(mem_type); H5Sclose(file_space); H5Sclose(mem_space);

		//TODO: check error state
		return 0;
	}

 	/** \func_read_hdr
 	*  Updates the content of passed **ptr** pointer, which must have enough memory space to receive data.
	*  Optional arguments **args:= h5::offset | h5::stride | h5::count | h5::block** may be specified for partial IO, 
	*  to describe the retrieved hyperslab from hdf5 file space. Default case is to select and retrieve all elements from dataset. 
	*  **h5::dxpl_t** provides control to datatransfer.
	* \code
	* h5::fd_t fd = h5::open("myfile.h5", H5F_ACC_RDWR);
	* h5::ds_t ds = h5::open(fd,"path/to/dataset");
	* std::vector<float> myvec(10*10);
	* auto err = h5::read( fd, "path/to/dataset", myvec.data(), h5::count{10,10}, h5::offset{5,0} );	
	* \endcode  
	* \par_file_path \par_dataset_path \par_ptr \par_offset \par_stride \par_count \par_block \par_dxpl \tpar_T \returns_err
 	*/ 
	template<class T, class... args_t>
	h5::herr_t read( hid_t fd, const std::string& dataset_path,T* ptr, args_t&&... args ){
		h5::ds_t ds = h5::open(fd, dataset_path );
		return h5::read<T>(ds, ptr, args...);
	}

 	/** \func_read_hdr
 	*  Updates the content of passed **ptr** pointer, which must have enough memory space to receive data.
	*  Optional arguments **args:= h5::offset | h5::stride | h5::count | h5::block** may be specified for partial IO,
	*  to describe the retrieved hyperslab from hdf5 file space. Default case is to select and retrieve all elements from dataset. 
	* \code
	* h5::fd_t fd = h5::open("myfile.h5", H5F_ACC_RDWR);
	* h5::ds_t ds = h5::open(fd,"path/to/dataset");
	* std::vector<float> myvec(10*10);
	* auto err = h5::read( fd, "path/to/dataset", myvec.data(), h5::count{10,10}, h5::offset{5,0} );	
	* \endcode  
	* \par_file_path \par_dataset_path \par_ptr \par_offset \par_stride \par_count \par_block \tpar_T \returns_err
 	*/ 
	template<class T, class... args_t>
	h5::herr_t read( const std::string& file_path, const std::string& dataset_path,T* ptr, args_t&&... args ){
		h5::fd_t fd = h5::open( file_path, H5F_ACC_RDWR, h5::default_fapl );
		return h5::read( fd, dataset_path, ptr, args...);
	}


/***************************  REFERENCE *****************************/
 	/** \func_read_hdr
 	*  Updates the content of passed **ref** reference, which must have enough memory space to receive data.
	*  Optional arguments **args:= h5::offset | h5::stride | h5::count | h5::block** may be specified for partial IO,
	*  to describe the retrieved hyperslab from hdf5 file space. Default case is to select and retrieve all elements from dataset. 
	* \code
	* h5::fd_t fd = h5::open("myfile.h5", H5F_ACC_RDWR);
	* h5::ds_t ds = h5::open(fd,"path/to/dataset");
	* std::vector<float> myvec(10*10);
	* auto err = h5::read( fd, "path/to/dataset", myvec, h5::offset{5,0} );	
	* \endcode  
	* \par_ds \par_ref \par_offset \par_stride  \par_block \tpar_T \returns_err
 	*/ 
	template<class T,  class... args_t>
		h5::herr_t read( const h5::ds_t& ds, T& ref, args_t&&... args ){
	// passed 'ref' contains memory size and element type, let's extract them
	// and delegate forward  
		using tcount  = typename arg::tpos<const h5::count_t&,const args_t&...>;
		using element_type    = typename utils::base<T>::type;

		static_assert( !tcount::present,
				"h5::count_t{ ... } is already present when passing arg by reference, did you mean to pass by pointer?" );
		// get 'count' and underlying type 
		h5::count_t count = utils::get_count(ref);
		element_type* ptr = utils::get_ptr(ref);
		return read<element_type>(ds, ptr, count, args...);
	}
 	/** \func_read_hdr
 	*  Updates the content of passed **ref** reference, which must have enough memory space to receive data.
	*  Optional arguments **args:= h5::offset | h5::stride | h5::count | h5::block** may be specified for partial IO, 
	*  to describe the retrieved hyperslab from hdf5 file space. Default case is to select and retrieve all elements from dataset. 
	* \code
	* std::vector<float> myvec(10*10);
	* h5::fd_t fd = h5::open("myfile.h5", H5F_ACC_RDWR);
	* auto err = h5::read( fd, "path/to/dataset", myvec, h5::offset{5,0} );	
	* \endcode  
	* \par_fd \par_dataset_path \par_ref \par_offset \par_stride  \par_block \tpar_T \returns_err
 	*/ 
	template<class T,  class... args_t> // dispatch to above
		h5::herr_t read( const h5::fd_t& fd,  const std::string& dataset_path, T& ref, args_t&&... args ){
		h5::ds_t ds = h5::open(fd, dataset_path );
		return h5::read<T>(ds, ref, args...);
	}
 	/** \func_read_hdr
 	*  Updates the content of passed **ref** reference, which must have enough memory space to receive data.
	*  Optional arguments **args:= h5::offset | h5::stride | h5::count | h5::block** may be specified for partial IO,
	*  to describe the retrieved hyperslab from  hdf5 file space. Default case is to select and retrieve all elements from dataset. 
	* \code
	* std::vector<float> myvec(10*10);
	* auto err = h5::read( "path/to/file.h5", "path/to/dataset", myvec, h5::offset{5,0} );	
	* \endcode  
	* \par_file_path \par_dataset_path \par_ref \par_offset \par_stride \par_block \tpar_T \returns_err
 	*/ 
	template<class T, class... args_t> // dispatch to above
	h5::herr_t read( const std::string& file_path, const std::string& dataset_path, T& ref, args_t&&... args ){
		h5::fd_t fd = h5::open( file_path, H5F_ACC_RDWR, h5::default_fapl );
		return h5::read( fd, dataset_path, ref, args...);
	}

/***************************  OBJECT *****************************/
 	/** \func_read_hdr
 	*  Direct read from an opened dataset descriptor that returns the entire data space wrapped into the object specified. 
	*  Optional arguments **args:= h5::offset | h5::stride | h5::count | h5::block** may be specified for partial IO, 
	*  to describe the retrieved hyperslab from hdf5 file space. Default case is to select and retrieve all elements from dataset. 
	* \code
	* h5::fd_t fd = h5::open("myfile.h5", H5F_ACC_RDWR);
	* auto vec = h5::read<std::vector<float>>( fd, "path/to/dataset",	h5::count{10,10}, h5::offset{5,0} );	
	* \endcode  
	* \par_ds \par_offset \par_stride \par_count \par_block \tpar_T \returns_object 
 	*/ 
	template<class T,  class... args_t>
	T read( const h5::ds_t& ds, args_t&&... args ){
	// if 'count' isn't specified use the one inside the hdf5 file, once it is obtained
	// collapse dimensions to the rank of the object returned and create this T object
	// update the content by we're good to go, since stride and offset can be processed in the 
	// update step
		using tcount  = typename arg::tpos<const h5::count_t&,const args_t&...>;
		using element_type    = typename utils::base<T>::type;

		h5::count_t size;
		const h5::count_t& count = arg::get(size, args...);
		h5::block_t  default_block{1,1,1,1,1,1,1};
		const h5::block_t& block = arg::get( default_block, args...);

		if constexpr( !tcount::present ){ // read count ::= current_dim of file space 
			h5::sp_t file_space = h5::sp_t{H5Dget_space(ds)};
			size.rank = H5Sget_simple_extent_dims(static_cast<hid_t>(file_space), *size, NULL);
		} else {
			for(int i=0;i<count.rank;i++) size[i] = count[i] * block[i];
			size.rank = count.rank;
		}
		T ref = utils::ctor<T>( count.rank, *size );
		element_type *ptr = utils::get_ptr( ref );

		h5::herr_t err = read<element_type>(ds, ptr, count, args...);
		return ref;
	}
 	/** \func_read_hdr
 	*  Direct read from an opened file descriptor and dataset path that returns the entire data space wrapped into the object specified. 
	*  Optional arguments **args:= h5::offset | h5::stride | h5::count | h5::block** may be specified in any order for partial IO, 
	*  to describe the retrieved hyperslab from hdf5 file space. Default case is to select and retrieve all elements from dataset. 
	* \code
	* h5::fd_t fd = h5::open("myfile.h5", H5F_ACC_RDWR);
	* auto vec = h5::read<std::vector<float>>( fd, "path/to/dataset",	h5::count{10,10}, h5::offset{5,0} );	
	* \endcode  
	* \par_fd \par_dataset_path \par_offset \par_stride \par_count \par_block \tpar_T \returns_object 
 	*/ 
	template<class T, class... args_t> // dispatch to above
	T read( hid_t fd, const std::string& dataset_path, args_t&&... args ){
		h5::ds_t ds = h5::open(fd, dataset_path );
		return h5::read<T>(ds, args...);
	}
 	/** \func_read_hdr
 	*  Direct read from file and dataset path that returns the entire data space wrapped into the object specified.
	*  Optional arguments **args:= h5::offset | h5:stride | h5::count | h5::block** may be specified for partial IO, to describe
	 *  the retrieved hyperslab from  hdf5 file space. Default case is to select and retrieve all elements from dataset.
	* \code
	* auto vec = h5::read<std::vector<float>>( "myfile.h5","path/to/dataset", h5::count{10,10}, h5::offset{5,0} );	
	* \endcode  
	* \par_file_path \par_dataset_path \par_offset \par_stride  \par_count \par_block \tpar_T \returns_object 
 	*/ 
	template<class T, class... args_t> // dispatch to above
	T read(const std::string& file_path, const std::string& dataset_path, args_t&&... args ){
		h5::fd_t fd = h5::open( file_path, H5F_ACC_RDWR, h5::default_fapl );
		return h5::read<T>( fd, dataset_path, args...);
	}
}
#endif
