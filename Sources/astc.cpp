#include "astc.h"
#include "../Libraries/astc-encoder/Source/astc_codec_internals.h"
#include <math.h>
#include <stdio.h>

#define DEBUG_ALLOW_ILLEGAL_BLOCK_SIZES 0

void store_astc_file(const astc_codec_image * input_image,
	const char *filename, int xdim, int ydim, int zdim, const error_weighting_params * ewp, astc_decode_mode decode_mode, swizzlepattern swz_encode, int threadcount);
void find_closest_blockdim_2d(float target_bitrate, int *x, int *y, int consider_illegal);
void find_closest_blockdim_3d(float target_bitrate, int *x, int *y, int *z, int consider_illegal);

namespace {
	void writeASTC(iw_image* image, const char* filename) {
		//int i;

		prepare_angular_tables();
		build_quantization_mode_table();

		astc_decode_mode decode_mode = DECODE_HDR;
		int opmode;					// 0=compress, 1=decompress, 2=do both, 4=compare
		opmode = 0;
		decode_mode = DECODE_LDR_SRGB;

		int array_size = 1;

		const char *output_filename = filename;

		int silentmode = 0;
		int timemode = 0;
		int psnrmode = 0;

		error_weighting_params ewp;

		ewp.rgb_power = 1.0f;
		ewp.alpha_power = 1.0f;
		ewp.rgb_base_weight = 1.0f;
		ewp.alpha_base_weight = 1.0f;
		ewp.rgb_mean_weight = 0.0f;
		ewp.rgb_stdev_weight = 0.0f;
		ewp.alpha_mean_weight = 0.0f;
		ewp.alpha_stdev_weight = 0.0f;

		ewp.rgb_mean_and_stdev_mixing = 0.0f;
		ewp.mean_stdev_radius = 0;
		ewp.enable_rgb_scale_with_alpha = 0;
		ewp.alpha_radius = 0;

		ewp.block_artifact_suppression = 0.0f;
		ewp.rgba_weights[0] = 1.0f;
		ewp.rgba_weights[1] = 1.0f;
		ewp.rgba_weights[2] = 1.0f;
		ewp.rgba_weights[3] = 1.0f;
		ewp.ra_normal_angular_scale = 0;

		swizzlepattern swz_encode = { 0, 1, 2, 3 };
		swizzlepattern swz_decode = { 0, 1, 2, 3 };


		int thread_count = 0;		// default value
		int thread_count_autodetected = 0;

		int preset_has_been_set = 0;

		int plimit_autoset = -1;
		int plimit_user_specified = -1;
		int plimit_set_by_user = 0;

		float dblimit_autoset_2d = 0.0;
		float dblimit_autoset_3d = 0.0;
		float dblimit_user_specified = 0.0;
		int dblimit_set_by_user = 0;

		float oplimit_autoset = 0.0;
		float oplimit_user_specified = 0.0;
		int oplimit_set_by_user = 0;

		float mincorrel_autoset = 0.0;
		float mincorrel_user_specified = 0.0;
		int mincorrel_set_by_user = 0;

		float bmc_user_specified = 0.0;
		float bmc_autoset = 0.0;
		int bmc_set_by_user = 0;

		int maxiters_user_specified = 0;
		int maxiters_autoset = 0;
		int maxiters_set_by_user = 0;

		int pcdiv = 1;

		int xdim_2d = 0;
		int ydim_2d = 0;
		int xdim_3d = 0;
		int ydim_3d = 0;
		int zdim_3d = 0;

		int target_bitrate_set = 0;
		float target_bitrate = 0;

		int print_block_mode_histogram = 0;

		float log10_texels_2d = 0.0f;
		float log10_texels_3d = 0.0f;

		int low_fstop = -10;
		int high_fstop = 10;


		// parse the commandline's encoding options.
		int argidx;
		const char* argv_encoding = "2.0";
		if (opmode == 0 || opmode == 2)
		{
			if (strchr(argv_encoding, '.') != NULL)
			{
				target_bitrate = static_cast < float >(atof(argv_encoding));
				target_bitrate_set = 1;
				find_closest_blockdim_2d(target_bitrate, &xdim_2d, &ydim_2d, DEBUG_ALLOW_ILLEGAL_BLOCK_SIZES);
				find_closest_blockdim_3d(target_bitrate, &xdim_3d, &ydim_3d, &zdim_3d, DEBUG_ALLOW_ILLEGAL_BLOCK_SIZES);
			}

			else
			{
				int dimensions = sscanf(argv_encoding, "%dx%dx%d", &xdim_3d, &ydim_3d, &zdim_3d);
				switch (dimensions)
				{
				case 0:
				case 1:
					// failed to parse the blocksize argument at all.
					printf("Blocksize not specified\n");
					exit(1);
				case 2:
				{
					zdim_3d = 1;

					// Check 2D constraints
					if (!(xdim_3d == 4 || xdim_3d == 5 || xdim_3d == 6 || xdim_3d == 8 || xdim_3d == 10 || xdim_3d == 12) ||
						!(ydim_3d == 4 || ydim_3d == 5 || ydim_3d == 6 || ydim_3d == 8 || ydim_3d == 10 || ydim_3d == 12))
					{
						printf("Block dimensions %d x %d unsupported\n", xdim_3d, ydim_3d);
						exit(1);
					}

					int is_legal_2d = (xdim_3d == ydim_3d) || (xdim_3d == ydim_3d + 1) || ((xdim_3d == ydim_3d + 2) && !(xdim_3d == 6 && ydim_3d == 4)) ||
						(xdim_3d == 8 && ydim_3d == 5) || (xdim_3d == 10 && ydim_3d == 5) || (xdim_3d == 10 && ydim_3d == 6);

					if (!DEBUG_ALLOW_ILLEGAL_BLOCK_SIZES && !is_legal_2d)
					{
						printf("Block dimensions %d x %d disallowed\n", xdim_3d, ydim_3d);
						exit(1);
					}
				}
				break;

				default:
				{
					// Check 3D constraints
					if (xdim_3d < 3 || xdim_3d > 6 || ydim_3d < 3 || ydim_3d > 6 || zdim_3d < 3 || zdim_3d > 6)
					{
						printf("Block dimensions %d x %d x %d unsupported\n", xdim_3d, ydim_3d, zdim_3d);
						exit(1);
					}

					int is_legal_3d = ((xdim_3d == ydim_3d) && (ydim_3d == zdim_3d)) || ((xdim_3d == ydim_3d + 1) && (ydim_3d == zdim_3d)) || ((xdim_3d == ydim_3d) && (ydim_3d == zdim_3d + 1));

					if (!DEBUG_ALLOW_ILLEGAL_BLOCK_SIZES && !is_legal_3d)
					{
						printf("Block dimensions %d x %d x %d disallowed\n", xdim_3d, ydim_3d, zdim_3d);
						exit(1);
					}
				}
				break;
				}

				xdim_2d = xdim_3d;
				ydim_2d = ydim_3d;
			}

			log10_texels_2d = log((float)(xdim_2d * ydim_2d)) / log(10.0f);
			log10_texels_3d = log((float)(xdim_3d * ydim_3d * zdim_3d)) / log(10.0f);
			argidx = 5;
		}
		else
		{
			// for decode and comparison, block size is not needed.
			argidx = 4;
		}

		silentmode = 1;

		// Alpha
		//ewp.enable_rgb_scale_with_alpha = 1;
		//ewp.alpha_radius = 1;
		
		// Exhaustive
		plimit_autoset = PARTITION_COUNT;
		oplimit_autoset = 1000.0f;
		mincorrel_autoset = 0.99f;
		dblimit_autoset_2d = 999.0f;
		dblimit_autoset_3d = 999.0f;
		bmc_autoset = 100;
		maxiters_autoset = 4;

		preset_has_been_set++;
		switch (ydim_2d)
		{
		case 4:
			pcdiv = 3;
			break;
		case 5:
			pcdiv = 1;
			break;
		case 6:
			pcdiv = 1;
			break;
		case 8:
			pcdiv = 1;
			break;
		case 10:
			pcdiv = 1;
			break;
		case 12:
			pcdiv = 1;
			break;
		default:
			pcdiv = 1;
			break;
		}
		
		// SRGB
		//perform_srgb_transform = 1;
		//dblimit_user_specified = 60;
		//dblimit_set_by_user = 1;
		
		float texel_avg_error_limit_2d = 0.0f;
		float texel_avg_error_limit_3d = 0.0f;

		if (opmode == 0 || opmode == 2)
		{
			// if encode, process the parsed commandline values

			//progress_counter_divider = pcdiv;

			int partitions_to_test = plimit_set_by_user ? plimit_user_specified : plimit_autoset;
			float dblimit_2d = dblimit_set_by_user ? dblimit_user_specified : dblimit_autoset_2d;
			float dblimit_3d = dblimit_set_by_user ? dblimit_user_specified : dblimit_autoset_3d;
			float oplimit = oplimit_set_by_user ? oplimit_user_specified : oplimit_autoset;
			float mincorrel = mincorrel_set_by_user ? mincorrel_user_specified : mincorrel_autoset;

			int maxiters = maxiters_set_by_user ? maxiters_user_specified : maxiters_autoset;
			ewp.max_refinement_iters = maxiters;

			ewp.block_mode_cutoff = (bmc_set_by_user ? bmc_user_specified : bmc_autoset) / 100.0f;

			if (rgb_force_use_of_hdr == 0)
			{
				texel_avg_error_limit_2d = pow(0.1f, dblimit_2d * 0.1f) * 65535.0f * 65535.0f;
				texel_avg_error_limit_3d = pow(0.1f, dblimit_3d * 0.1f) * 65535.0f * 65535.0f;
			}
			else
			{
				texel_avg_error_limit_2d = 0.0f;
				texel_avg_error_limit_3d = 0.0f;
			}
			ewp.partition_1_to_2_limit = oplimit;
			ewp.lowest_correlation_cutoff = mincorrel;

			if (partitions_to_test < 1)
				partitions_to_test = 1;
			else if (partitions_to_test > PARTITION_COUNT)
				partitions_to_test = PARTITION_COUNT;
			ewp.partition_search_limit = partitions_to_test;

			// if diagnostics are run, force the thread count to 1.
			if (
				print_tile_errors > 0 || print_statistics > 0)
			{
				thread_count = 1;
				thread_count_autodetected = 0;
			}

			if (thread_count < 1)
			{
				thread_count = 1; // get_number_of_cpus();
				thread_count_autodetected = 1;
			}


			// Specifying the error weight of a color component as 0 is not allowed.
			// If weights are 0, then they are instead set to a small positive value.

			float max_color_component_weight = MAX(MAX(ewp.rgba_weights[0], ewp.rgba_weights[1]),
				MAX(ewp.rgba_weights[2], ewp.rgba_weights[3]));
			ewp.rgba_weights[0] = MAX(ewp.rgba_weights[0], max_color_component_weight / 1000.0f);
			ewp.rgba_weights[1] = MAX(ewp.rgba_weights[1], max_color_component_weight / 1000.0f);
			ewp.rgba_weights[2] = MAX(ewp.rgba_weights[2], max_color_component_weight / 1000.0f);
			ewp.rgba_weights[3] = MAX(ewp.rgba_weights[3], max_color_component_weight / 1000.0f);
		}


		int padding = MAX(ewp.mean_stdev_radius, ewp.alpha_radius);

		// determine encoding bitness as follows:
		// if enforced by the output format, follow the output format's result
		// else use decode_mode to pick bitness.
		int bitness = get_output_filename_enforced_bitness(output_filename);
		if (bitness == -1)
		{
			bitness = (decode_mode == DECODE_HDR) ? 16 : 8;
		}

		int xdim = -1;
		int ydim = -1;
		int zdim = -1;

		// Temporary image array (for merging multiple 2D images into one 3D image).
		int *load_results = NULL;
		astc_codec_image **input_images = NULL;

		int load_result = 0;
		astc_codec_image *input_image = NULL;
		astc_codec_image *output_image = NULL;
		int input_components = 0;

		int input_image_is_hdr = 0;

		// load image
		if (opmode == 0 || opmode == 2 || opmode == 3)
		{
			// Allocate arrays for image data and load results.
			load_results = new int[array_size];
			input_images = new astc_codec_image *[array_size];

			// Iterate over all input images.
			/*for (int image_index = 0; image_index < array_size; image_index++)
			{
				// 2D input data.
				if (array_size == 1)
				{
					input_images[image_index] = astc_codec_load_image(input_filename, padding, &load_results[image_index]);
				}

				// 3D input data - multiple 2D images.
				else
				{
					char new_input_filename[256];

					// Check for extension: <name>.<extension>
					if (NULL == strrchr(input_filename, '.'))
					{
						printf("Unable to determine file type from extension: %s\n", input_filename);
						exit(1);
					}

					// Construct new file name and load: <name>_N.<extension>
					strcpy(new_input_filename, input_filename);
					sprintf(strrchr(new_input_filename, '.'), "_%d%s", image_index, strrchr(input_filename, '.'));
					input_images[image_index] = astc_codec_load_image(new_input_filename, padding, &load_results[image_index]);

					// Check image is not 3D.
					if (input_images[image_index]->zsize != 1)
					{
						printf("3D source images not supported with -array option: %s\n", new_input_filename);
						exit(1);
					}

					// BCJ(DEBUG)
					// printf("\n\n Image %d \n", image_index);
					// dump_image( input_images[image_index] );
					// printf("\n\n");
				}

				// Check load result.
				if (load_results[image_index] < 0)
				{
					printf("Failed to load image %s\n", input_filename);
					exit(1);
				}

				// Check format matches other slices.
				if (load_results[image_index] != load_results[0])
				{
					printf("Mismatching image format - image 0 and %d are a different format\n", image_index);
					exit(1);
				}
			}

			load_result = load_results[0];*/

			// Assign input image.
			//input_image = input_images[0];
			input_image->padding = padding;
			input_image->imagedata8 = (uint8_t***)image->pixels;
			input_image->imagedata16 = NULL;
			input_image->xsize = image->width;
			input_image->ysize = image->height;
			input_image->zsize = 1;
			
			input_components = load_result & 7;
			input_image_is_hdr = (load_result & 0x80) ? 1 : 0;

			if (input_image->zsize > 1)
			{
				xdim = xdim_3d;
				ydim = ydim_3d;
				zdim = zdim_3d;
				ewp.texel_avg_error_limit = texel_avg_error_limit_3d;
			}
			else
			{
				xdim = xdim_2d;
				ydim = ydim_2d;
				zdim = 1;
				ewp.texel_avg_error_limit = texel_avg_error_limit_2d;
			}
			expand_block_artifact_suppression(xdim, ydim, zdim, &ewp);

			if (padding > 0 || ewp.rgb_mean_weight != 0.0f || ewp.rgb_stdev_weight != 0.0f || ewp.alpha_mean_weight != 0.0f || ewp.alpha_stdev_weight != 0.0f)
			{
				compute_averages_and_variances(input_image, ewp.rgb_power, ewp.alpha_power, ewp.mean_stdev_radius, ewp.alpha_radius, swz_encode);
			}
		}

		if (opmode == 0)
		{
			store_astc_file(input_image, output_filename, xdim, ydim, zdim, &ewp, decode_mode, swz_encode, thread_count);
		}
	}
}

void astc(iw_context* context, const char* filename) {
	//int w = iw_get_value(context, IW_VAL_INPUT_WIDTH);
	//int h = iw_get_value(context, IW_VAL_INPUT_HEIGHT);
	//scale(context, getPower2(w), getPower2(h));
	iw_image img;
	iw_get_output_image(context, &img);
	writeASTC(&img, filename);
}
