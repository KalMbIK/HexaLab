#include <low_quality_view.h>

namespace HexaLab {
    void LowQualityView::set_mesh(js_ptr mesh_ptr) {
        this->mesh = (Mesh*)mesh_ptr;
        // TODO compute quality here instead of inside the builder? and split quality from hexa?
        edge_marks.resize(mesh->edges.size());
    }

    void LowQualityView::add_lowq_wireframe(Dart& dart) {
        MeshNavigator nav = mesh->navigate(dart);
        if (nav.edge().mark != mark) {
            nav.edge().mark = mark;
            for (int v = 0; v < 2; ++v) {
                lowq_model.surface_vert_pos.push_back(mesh->verts[nav.dart().vert].position);
                nav = nav.flip_vert();
            }
        }
    }

    void LowQualityView::add_hidden_wireframe(Dart& dart) {
        MeshNavigator nav = mesh->navigate(dart);
        if (nav.edge().mark != mark) {
            nav.edge().mark = mark;
            for (int v = 0; v < 2; ++v) {
                hidden_model.surface_vert_pos.push_back(mesh->verts[nav.dart().vert].position);
                nav = nav.flip_vert();
            }
        }
    }

    void LowQualityView::add_lowq_face(Dart& dart, float normal_sign) {
        MeshNavigator nav = mesh->navigate(dart);

        for (int i = 0; i < 2; ++i) {
            int j = 0;
            for (; j < 2; ++j) {
                lowq_model.surface_vert_pos.push_back(mesh->verts[nav.dart().vert].position);
                add_lowq_wireframe(nav.dart());
                nav = nav.rotate_on_face();
            }
            lowq_model.surface_vert_pos.push_back(mesh->verts[nav.dart().vert].position);

            Vector3f normal = nav.face().normal * normal_sign;
            lowq_model.surface_vert_norm.push_back(normal);
            lowq_model.surface_vert_norm.push_back(normal);
            lowq_model.surface_vert_norm.push_back(normal);

            Vector3f color = Vector3f(1 - nav.hexa().scaled_jacobian, 0, nav.hexa().scaled_jacobian);
            lowq_model.surface_vert_color.push_back(color);
            lowq_model.surface_vert_color.push_back(color);
            lowq_model.surface_vert_color.push_back(color);
        }
    }

    void LowQualityView::add_hidden_face(Dart& dart) {
        MeshNavigator nav = mesh->navigate(dart);

        for (int i = 0; i < 2; ++i) {
            int j = 0;
            for (; j < 2; ++j) {
                hidden_model.surface_vert_pos.push_back(mesh->verts[nav.dart().vert].position);
                add_hidden_wireframe(nav.dart());
                nav = nav.rotate_on_face();
            }
            hidden_model.surface_vert_pos.push_back(mesh->verts[nav.dart().vert].position);

            Vector3f normal = nav.face().normal;
            hidden_model.surface_vert_norm.push_back(normal);
            hidden_model.surface_vert_norm.push_back(normal);
            hidden_model.surface_vert_norm.push_back(normal);
        }
    }

    void LowQualityView::update() {
        ++mark;

        lowq_model.clear();
        hidden_model.clear();

        auto t_draw = sample_time();
        for (size_t i = 0; i < mesh->faces.size(); ++i) {
            MeshNavigator nav = mesh->navigate(mesh->faces[i]);
            // hexa a visible, hexa b not existing or not visible
            if (nav.hexa().scaled_jacobian <= quality_threshold && (nav.dart().hexa_neighbor == -1 || nav.flip_hexa().hexa().scaled_jacobian > quality_threshold)) {
                add_lowq_face(nav.dart(), 1);
                // hexa a invisible, hexa b existing and visible
            } else if (nav.hexa().scaled_jacobian > quality_threshold && nav.dart().hexa_neighbor != -1 && nav.flip_hexa().hexa().scaled_jacobian <= quality_threshold) {
                nav = nav.flip_hexa().flip_edge();
                add_lowq_face(nav.dart(), -1);
                // face was culled by the plane, is surface
            } else if (nav.hexa().scaled_jacobian > quality_threshold && (nav.dart().hexa_neighbor == -1 || nav.flip_hexa().hexa().scaled_jacobian <= quality_threshold)) {
                add_hidden_face(nav.dart());
            }
        }
        auto dt_draw = milli_from_sample(t_draw);
        HL_LOG("[%s] Face pass took %dms", name, dt_draw);
    }
}