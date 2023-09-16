#ifndef LIBOCEANLIGHT_ENGINE_HPP_INCLUDED
#define LIBOCEANLIGHT_ENGINE_HPP_INCLUDED
#include <vector>
#include <array>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <liboceanlight/lol_window.hpp>
#include <config.h>

namespace liboceanlight
{
	namespace engine
	{
		typedef struct lol_engine_data_struct
		{
			/* INSTANCE */
			VkDebugUtilsMessengerEXT dbg_messenger {nullptr};
			VkInstance vulkan_instance {nullptr};
#ifdef NDEBUG
			const bool validation_layer_enabled {false};
#else
			const bool validation_layer_enabled {true};
#endif /* NDEBUG */

			/* DEVICE */
			VkPhysicalDevice physical_device {VK_NULL_HANDLE};
			VkDevice logical_device {VK_NULL_HANDLE};
			static constexpr std::array dev_extensions {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME};

			/* SURFACE */
			VkSurfaceKHR window_surface {nullptr};
			VkSurfaceCapabilitiesKHR capabilities {};
			VkSurfaceFormatKHR surface_format {};

			/* QUEUE */
			uint32_t graphics_queue_index {UINT32_MAX};
			VkQueue graphics_queue {nullptr};

			/* SWAPCHAIN */
			VkSwapchainKHR swap_chain {nullptr};
			VkExtent2D swap_extent {};
			VkPresentModeKHR present_mode;
			std::vector<VkImage> images;
			std::vector<VkImageView> image_views;
			std::vector<VkFramebuffer> frame_buffers;

			/* PIPELINE */
			VkDescriptorSetLayout descriptor_set_layout {nullptr};
			VkPipelineLayout pipeline_layout {nullptr};
			VkRenderPass render_pass {nullptr};
			VkPipeline graphics_pipeline {nullptr};

			/* COMMAND */
			static constexpr int max_frames_in_flight {2};
			VkCommandPool command_pool {nullptr};
			std::array<VkCommandBuffer, max_frames_in_flight> command_buffers;

			/* DRAW */
			int current_frame {0};
			std::array<VkSemaphore, max_frames_in_flight> signal_sems;
			std::array<VkSemaphore, max_frames_in_flight> wait_sems;
			std::array<VkFence, max_frames_in_flight> in_flight_fences;

			/* VERTEX BUFFER */
			VkBuffer vertex_buffer {nullptr};
			VkDeviceMemory vertex_buffer_mem {nullptr};

			/* INDEX BUFFER */
			VkBuffer index_buffer {nullptr};
			VkDeviceMemory index_buffer_mem {nullptr};

			/* UNIFORM BUFFER */
			std::array<VkBuffer, max_frames_in_flight> uniform_buffers;
			std::array<VkDeviceMemory, max_frames_in_flight>
				uniform_buffers_mem;
			std::array<void*, max_frames_in_flight> uniform_buffers_mapped;

			/* DESCRIPTOR */
			VkDescriptorPool descriptor_pool {nullptr};
			std::array<VkDescriptorSet, max_frames_in_flight> descriptor_sets;
		} engine_data;

		typedef struct lol_vertex_struct
		{
			glm::vec2 pos;
			glm::vec3 color;

			static VkVertexInputBindingDescription get_binding_desc()
			{
				VkVertexInputBindingDescription binding_desc {};
				binding_desc.binding = 0;
				binding_desc.stride = sizeof(lol_vertex_struct);
				binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				return binding_desc;
			};

			static std::array<VkVertexInputAttributeDescription, 2>
			get_attribute_descs()
			{
				std::array<VkVertexInputAttributeDescription, 2>
					attribute_descs {};

				attribute_descs[0].binding = 0;
				attribute_descs[0].location = 0;
				attribute_descs[0].format = VK_FORMAT_R32G32_SFLOAT;
				attribute_descs[0].offset = offsetof(lol_vertex_struct, pos);

				attribute_descs[1].binding = 0;
				attribute_descs[1].location = 1;
				attribute_descs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
				attribute_descs[1].offset = offsetof(lol_vertex_struct, color);
				return attribute_descs;
			};
		} vertex;

		/* array of structs */
		static std::vector<vertex> vertices {
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};
		static std::array<uint16_t, 6> indices {0, 1, 2, 2, 3, 0};
		struct uniform_buffer_object
		{
			glm::mat4 model, view, proj;
		};

		void start(liboceanlight::window&, engine_data&);
		void run(liboceanlight::window&, engine_data&);
		void draw_frame(liboceanlight::window&, engine_data&);
		void record_cmd_buffer(engine_data&, VkCommandBuffer&, uint32_t);
		void recreate_swapchain(liboceanlight::window&, engine_data&);
		void upload_buffer(engine_data&,
						   void*,
						   VkDeviceSize,
						   VkBufferUsageFlagBits,
						   VkBuffer&,
						   VkDeviceMemory&);
		void update_uniform_buffer(engine_data&, uint32_t);
	} /* namespace engine */
} /* namespace liboceanlight */
#endif /* LIBOCEANLIGHT_ENGINE_HPP_INCLUDED */
