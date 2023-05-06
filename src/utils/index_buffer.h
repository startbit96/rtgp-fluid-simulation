#pragma once

class Index_Buffer
{
    private:
        unsigned int m_renderer_id;
        unsigned int m_count;

    public:
        Index_Buffer(const unsigned int* indices, unsigned int count);
        ~Index_Buffer();

        void bind() const;
        void unbind() const;

        inline unsigned int get_count() const { return m_count; }
};