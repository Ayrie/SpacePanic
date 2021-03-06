#include "SpriteBatch.h"
#include <algorithm>

namespace GameEngine{

	Glypth::Glypth(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint Texture, float Depth, const ColorRGBA8& color) : texture(Texture), depth(Depth){

		topLeft.color = color;
		topLeft.setPosition(destRect.x, destRect.y + destRect.w);
		topLeft.setUV(uvRect.x, uvRect.y + uvRect.w);

		bottomLeft.color = color;
		bottomLeft.setPosition(destRect.x, destRect.y);
		bottomLeft.setUV(uvRect.x, uvRect.y);

		topRight.color = color;
		topRight.setPosition(destRect.x + destRect.z, destRect.y + destRect.w);
		topRight.setUV(uvRect.x + uvRect.z, uvRect.y + uvRect.w);

		bottomRight.color = color;
		bottomRight.setPosition(destRect.x + destRect.z, destRect.y);
		bottomRight.setUV(uvRect.x + uvRect.z, uvRect.y);
	}

	Glypth::Glypth(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint Texture, float Depth, const ColorRGBA8& color, float angle) : texture(Texture), depth(Depth){

		glm::vec2 halfDimensions(destRect.z / 2.0f, destRect.w / 2.0f);

		//Get points centered at origin
		glm::vec2 tl(-halfDimensions.x, halfDimensions.y);
		glm::vec2 bl(-halfDimensions.x, -halfDimensions.y);

		glm::vec2 br(halfDimensions.x, -halfDimensions.y);
		glm::vec2 tr(halfDimensions.x, halfDimensions.y);

		//Rotate the points
		tl = rotatePoint(tl, angle) + halfDimensions;
		bl = rotatePoint(bl, angle) + halfDimensions;
		br = rotatePoint(br, angle) + halfDimensions;
		tr = rotatePoint(tr, angle) + halfDimensions;

		topLeft.color = color;
		topLeft.setPosition(destRect.x + tl.x, destRect.y + tl.y);
		topLeft.setUV(uvRect.x, uvRect.y + uvRect.w);

		bottomLeft.color = color;
		bottomLeft.setPosition(destRect.x + bl.x, destRect.y + bl.y);
		bottomLeft.setUV(uvRect.x, uvRect.y);

		topRight.color = color;
		topRight.setPosition(destRect.x + tr.x, destRect.y + tr.y);
		topRight.setUV(uvRect.x + uvRect.z, uvRect.y + uvRect.w);

		bottomRight.color = color;
		bottomRight.setPosition(destRect.x + br.x, destRect.y + br.y);
		bottomRight.setUV(uvRect.x + uvRect.z, uvRect.y);

	}

	glm::vec2 Glypth::rotatePoint(glm::vec2 position, float angle){
		glm::vec2 newVector;
		newVector.x = position.x * cos(angle) - position.y * sin(angle);
		newVector.y = position.x * sin(angle) + position.y * cos(angle);

		return newVector;
	}



	SpriteBatch::SpriteBatch(void) : m_vbo(0), m_vao(0)
	{
	}


	SpriteBatch::~SpriteBatch(void)
	{
	}


	void SpriteBatch::init(){
		createVertexArray();

	}

	void SpriteBatch::begin(GlypthSortType sortType) /* = GlypthSortType::TEXTURE) */{
		m_sortType = sortType;
		m_renderBatches.clear();
		m_glypths.clear();
	}

	void SpriteBatch::end(){
		//Set up all pointers for fast sorting
		m_glypthPointers.resize(m_glypths.size());
		for (int i = 0; i < m_glypths.size(); i++)
		{
			m_glypthPointers[i] = &m_glypths[i];
		}
		sortGlypths();
		createRenderBatches();

	}

	void SpriteBatch::draw(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const ColorRGBA8& color){

		m_glypths.emplace_back(destRect, uvRect, texture, depth, color);
	}


	void SpriteBatch::draw(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const ColorRGBA8& color, float angle) {
		m_glypths.emplace_back(destRect, uvRect, texture, depth, color, angle);
	}

	void SpriteBatch::draw(const glm::vec4& destRect, const glm::vec4& uvRect, GLuint texture, float depth, const ColorRGBA8& color, glm::vec2& dir) { //dir = direction
		
		const glm::vec2 right(1.0f, 0.0f);
		float angle = acos(glm::dot(right, dir));

		if (dir.y < 0.0f)
		{
			angle = -angle;
		}

		m_glypths.emplace_back(destRect, uvRect, texture, depth, color, angle);
	}



	void SpriteBatch::renderBatch(){

		glBindVertexArray(m_vao);

		for (int i = 0; i < m_renderBatches.size(); i++)
		{
			glBindTexture(GL_TEXTURE_2D, m_renderBatches[i].texture);

			glDrawArrays(GL_TRIANGLES, m_renderBatches[i].offset, m_renderBatches[i].numVertices);
		}

		glBindVertexArray(0);
	}

	void SpriteBatch::createRenderBatches(){
		std::vector<Vertex> vertices;
		vertices.resize(m_glypthPointers.size() * 6);

		if (m_glypthPointers.empty())
		{
			return;
		}

		int offset = 0;
		int cv = 0; //current vertex

		m_renderBatches.emplace_back(offset, 6, m_glypthPointers[0]->texture);
		vertices[cv++] = m_glypthPointers[0]->topLeft;
		vertices[cv++] = m_glypthPointers[0]->bottomLeft;
		vertices[cv++] = m_glypthPointers[0]->bottomRight;
		vertices[cv++] = m_glypthPointers[0]->bottomRight;
		vertices[cv++] = m_glypthPointers[0]->topRight;
		vertices[cv++] = m_glypthPointers[0]->topLeft;
		offset += 6;

		for (int cg = 1; cg < m_glypthPointers.size(); cg++) //cg = current Glypth
		{
			if (m_glypthPointers[cg]->texture != m_glypthPointers[cg - 1]->texture)
			{
				m_renderBatches.emplace_back(offset, 6, m_glypthPointers[cg]->texture);
			}
			else{
				m_renderBatches.back().numVertices += 6;
			}
			vertices[cv++] = m_glypthPointers[cg]->topLeft;
			vertices[cv++] = m_glypthPointers[cg]->bottomLeft;
			vertices[cv++] = m_glypthPointers[cg]->bottomRight;
			vertices[cv++] = m_glypthPointers[cg]->bottomRight;
			vertices[cv++] = m_glypthPointers[cg]->topRight;
			vertices[cv++] = m_glypthPointers[cg]->topLeft;
			offset += 6;
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		//glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW); faster way -->
		//orphan the buffer
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
		//upload the data
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());

		glBindBuffer(GL_ARRAY_BUFFER, 0);




	}

	void SpriteBatch::createVertexArray(){

		if (m_vao == 0)
		{
			glGenVertexArrays(1, &m_vao);
		}

		glBindVertexArray(m_vao);

		if (m_vbo == 0)
		{
			glGenBuffers(1, &m_vbo);
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);


		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		//tells openGl where the data is
		//this is the position attribute pointer
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

		//This is the color attribute pointer
		glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));

		//This is the UV attribute pointer
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

		glBindVertexArray(0);

	}

	void SpriteBatch::sortGlypths(){
		switch (m_sortType)
		{
		case GameEngine::GlypthSortType::FRONT_TO_BACK:
			std::stable_sort(m_glypthPointers.begin(), m_glypthPointers.end(), compareFrontToBack);
			break;
		case GameEngine::GlypthSortType::BACK_TO_FRONT:
			std::stable_sort(m_glypthPointers.begin(), m_glypthPointers.end(), compareFrontToBack);
			break;
		case GameEngine::GlypthSortType::TEXTURE:
			std::stable_sort(m_glypthPointers.begin(), m_glypthPointers.end(), compareTexture);
			break;
		default:
			break;
		}
	}

	bool SpriteBatch::compareFrontToBack(Glypth* a, Glypth* b){
		return (a->depth < b->depth);
	}

	bool SpriteBatch::compareBackToFront(Glypth* a, Glypth* b){
		return (a->depth > b->depth);
	}

	bool SpriteBatch::compareTexture(Glypth* a, Glypth* b){
		return (a->texture < b->texture);
	}


}