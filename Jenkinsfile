pipeline {
    agent {
        docker {
            image 'aperture_standard'
        }
    }
    stages {
        stage('configure') {
              steps {
                  sh '''
                      if [ -d build ];
                      then rm  -Rf build;
                      fi
                      mkdir build
                      meson build
                  '''
                  dir(path: 'build') {
                      sh '''
                          meson configure -Db_coverage=true
                      '''
                  }
              }
        }
        stage('build') {
            steps {
                dir(path: 'build') {
                    sh '''
                        ninja test
                        ninja coverage-xml
                        ninja coverage-html
                        gcov -i -j `find . -iname *.gcno`
                    '''
                }
            }
        }
        stage('package') {
            steps {
                archiveArtifacts(artifacts: 'build/*', allowEmptyArchive: true, caseSensitive: true, onlyIfSuccessful: true)
            }
        }
    }
}
