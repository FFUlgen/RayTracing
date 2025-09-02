#include "Renderer.h"
#include "Walnut/Random.h"

namespace Utils
{
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);
		
		uint32_t result = (a << 24) | (b << 16) | (g << 8) | (r);
		return result;
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];

}

void Renderer::Render()
{
	// render every pixel
	/*
	for (uint32_t i = 0; i < m_FinalImage->GetWidth() * m_FinalImage->GetHeight(); i++)
	{
		glm::vec2 coord = { , i / m_FinalImage->GetWidth() };
		m_ImageData[i] = PerPixel(coord);
		//m_ImageData[i] = 0xffff00ff; Magenta
		m_ImageData[i] = Walnut::Random::UInt();
		m_ImageData[i] |= 0xff000000;
	}
	*/
	
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec2 coord = { (float)x / m_FinalImage->GetWidth(), (float)y / m_FinalImage->GetHeight() };
			coord = coord * 2.0f - 1.0f; // coord value mapped to [-1,1]
			float ar =  (float)m_FinalImage->GetWidth() / m_FinalImage->GetHeight();
			coord.x *= ar;
			glm::vec4 color = PerPixel(coord);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}
	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::PerPixel(glm::vec2 coord)
{
	//uint8_t r = (uint8_t)(coord.x * 255.0f);
	//uint8_t g = (uint8_t)(coord.y * 255.0f);
	
	glm::vec3 rayOrigin(0.0f, 0.0f, 1.0f);
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
	float radius = 0.5f;
	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0;
	// where
	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit distance
	
	// ax^2 + bx + c
	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;
	
	float discriminant = b * b - 4.0f * a * c;
	if (discriminant < 0.0f)
	{
		return glm::vec4(0, 0, 0, 1);
	}

	float t1 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
	float t2 = (-b - glm::sqrt(discriminant)) / (2.0f * a); // since a is always positive t2 must be the closest hit point
	glm::vec3 h1 = rayOrigin + rayDirection * t1; // Position of hit 1
	glm::vec3 h2 = rayOrigin + rayDirection * t2; // Position of hit 2
	// Normally we should subtract sphere origin from hit point but we know sphere origin is (0, 0, 0) so we skip that step.
	glm::vec3 normal = glm::normalize(h2); 
	glm::vec3 lightDirection = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
	float d = glm::max(glm::dot(normal, -lightDirection), 0.0f); // == cos(angle);
	glm::vec3 sphereColor(1, 0, 1);
	//sphereColor = normal * 0.5f + 0.5f; // we remapped range to [0,1] since this is color value
	sphereColor *= d;
	return glm::vec4(sphereColor, 1.0f);

}

